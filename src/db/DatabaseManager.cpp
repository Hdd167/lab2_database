#include "DatabaseManager.h"

#include "BinaryIO.h"
#include "DbException.h"

#include <QByteArray>
#include <QFileInfo>
#include <QtMath>

namespace {

constexpr char DB_EXT[] = ".dat";
constexpr char IDX_EXT[] = ".idx";

}

namespace db {

DatabaseManager::~DatabaseManager()
{
    try {
        close();
    } catch (const DbException &) {

    }
}

void DatabaseManager::create(const QString &basePath)
{
    close();
    m_basePath = basePath;
    openFiles(basePath, true);
}

void DatabaseManager::open(const QString &basePath)
{
    close();
    if (!QFileInfo::exists(basePath + QLatin1String(DB_EXT))) {
        throw DbException(
            QStringLiteral("Database not found: %1%2").arg(basePath, QLatin1String(DB_EXT)));
    }
    m_basePath = basePath;
    openFiles(basePath, false);
}

void DatabaseManager::openFiles(const QString &basePath, bool create)
{
    m_dbFile.setFileName(basePath + QLatin1String(DB_EXT));
    if (!m_dbFile.open(QIODevice::ReadWrite)) {
        throw DbException(QStringLiteral("Cannot open database file %1: %2")
                              .arg(m_dbFile.fileName(), m_dbFile.errorString()));
    }
    if (create && !m_dbFile.resize(0)) {
        throw DbException(QStringLiteral("Cannot create database file %1: %2")
                              .arg(m_dbFile.fileName(), m_dbFile.errorString()));
    }
    m_index = std::make_unique<HashIndex>(basePath + QLatin1String(IDX_EXT), create);
}

void DatabaseManager::close()
{
    if (m_dbFile.isOpen()) {
        m_dbFile.close();
    }
    if (m_index) {
        m_index->close();
        m_index.reset();
    }
}

void DatabaseManager::clear()
{
    ensureOpen();
    if (!m_dbFile.resize(0)) {
        throw DbException(
            QStringLiteral("Cannot truncate database file: %1").arg(m_dbFile.errorString()));
    }
    m_index->clear();
}

void DatabaseManager::deleteDatabase()
{
    const QString base = m_basePath;
    close();
    m_basePath.clear();
    if (!base.isEmpty()) {
        QFile::remove(base + QLatin1String(DB_EXT));
        QFile::remove(base + QLatin1String(IDX_EXT));
    }
}

void DatabaseManager::addRecord(const Record &record)
{
    ensureOpen();
    const qint64 offset = m_dbFile.size();
    writeRecord(record, offset);
    try {
        m_index->insert(record.id, offset);
    } catch (const DuplicateKeyException &) {
        char deleted = 1;
        if (!m_dbFile.seek(offset + Record::RECORD_SIZE - 1)
            || m_dbFile.write(&deleted, 1) != 1 || !m_dbFile.flush()) {
            throw DbException(
                QStringLiteral("Cannot write database file: %1").arg(m_dbFile.errorString()));
        }
        throw;
    }
}

std::optional<Record> DatabaseManager::findByKey(int id) const
{
    ensureOpen();
    const qint64 offset = m_index->find(id);
    if (offset < 0) {
        return std::nullopt;
    }
    return readRecord(offset);
}

QList<Record> DatabaseManager::findByField(const QString &field, const QString &value) const
{
    ensureOpen();
    return scanRecords([&field, &value](const Record &record) {
        return matchesField(record, field, value);
    });
}

bool DatabaseManager::deleteByKey(int id)
{
    ensureOpen();
    const qint64 offset = m_index->find(id);
    if (offset < 0) {
        return false;
    }
    char deleted = 1;
    if (!m_dbFile.seek(offset + Record::RECORD_SIZE - 1) || m_dbFile.write(&deleted, 1) != 1
        || !m_dbFile.flush()) {
        throw DbException(
            QStringLiteral("Cannot write database file: %1").arg(m_dbFile.errorString()));
    }
    m_index->remove(id);
    return true;
}

int DatabaseManager::deleteByField(const QString &field, const QString &value)
{
    ensureOpen();
    int count = 0;
    const qint64 length = m_dbFile.size();
    for (qint64 offset = 0; offset + Record::RECORD_SIZE <= length;
         offset += Record::RECORD_SIZE) {
        const Record record = readRecord(offset);
        if (!record.deleted && matchesField(record, field, value)) {
            char deleted = 1;
            if (!m_dbFile.seek(offset + Record::RECORD_SIZE - 1)
                || m_dbFile.write(&deleted, 1) != 1 || !m_dbFile.flush()) {
                throw DbException(QStringLiteral("Cannot write database file: %1")
                                      .arg(m_dbFile.errorString()));
            }
            m_index->remove(record.id);
            ++count;
        }
    }
    return count;
}

bool DatabaseManager::editRecord(int id, const Record &newData)
{
    ensureOpen();
    const qint64 offset = m_index->find(id);
    if (offset < 0) {
        return false;
    }
    Record record = newData;
    record.id = id;
    record.deleted = false;
    writeRecord(record, offset);
    return true;
}

QList<Record> DatabaseManager::getAllRecords() const
{
    ensureOpen();
    return scanRecords([](const Record &) { return true; });
}

void DatabaseManager::backup(const QString &backupBase) const
{
    ensureOpen();
    copyFile(m_basePath + QLatin1String(DB_EXT), backupBase + QLatin1String(DB_EXT));
    copyFile(m_basePath + QLatin1String(IDX_EXT), backupBase + QLatin1String(IDX_EXT));
}

void DatabaseManager::restore(const QString &backupBase)
{
    ensureOpen();
    const QString current = m_basePath;
    close();
    copyFile(backupBase + QLatin1String(DB_EXT), current + QLatin1String(DB_EXT));
    copyFile(backupBase + QLatin1String(IDX_EXT), current + QLatin1String(IDX_EXT));
    open(current);
}

void DatabaseManager::writeRecord(const Record &record, qint64 offset)
{
    char buffer[Record::RECORD_SIZE];
    int pos = 0;

    putInt(buffer + pos, record.id);
    pos += 4;
    writeString(buffer + pos, record.name, Record::NAME_MAX_LEN);
    pos += 2 * Record::NAME_MAX_LEN;
    putDouble(buffer + pos, record.price);
    pos += 8;
    putInt(buffer + pos, record.quantity);
    pos += 4;
    writeString(buffer + pos, record.category, Record::CATEGORY_MAX_LEN);
    pos += 2 * Record::CATEGORY_MAX_LEN;
    putByte(buffer + pos, record.inStock ? 1 : 0);
    ++pos;
    putByte(buffer + pos, record.deleted ? 1 : 0);

    if (!m_dbFile.seek(offset) || m_dbFile.write(buffer, Record::RECORD_SIZE) != Record::RECORD_SIZE
        || !m_dbFile.flush()) {
        throw DbException(
            QStringLiteral("Cannot write database file: %1").arg(m_dbFile.errorString()));
    }
}

Record DatabaseManager::readRecord(qint64 offset) const
{
    char buffer[Record::RECORD_SIZE];
    if (!m_dbFile.seek(offset) || m_dbFile.read(buffer, Record::RECORD_SIZE) != Record::RECORD_SIZE) {
        throw DbException(
            QStringLiteral("Database file is damaged or truncated: %1").arg(m_dbFile.fileName()));
    }

    Record record;
    int pos = 0;

    record.id = getInt(buffer + pos);
    pos += 4;
    record.name = readString(buffer + pos, Record::NAME_MAX_LEN);
    pos += 2 * Record::NAME_MAX_LEN;
    record.price = getDouble(buffer + pos);
    pos += 8;
    record.quantity = getInt(buffer + pos);
    pos += 4;
    record.category = readString(buffer + pos, Record::CATEGORY_MAX_LEN);
    pos += 2 * Record::CATEGORY_MAX_LEN;
    record.inStock = getByte(buffer + pos) == 1;
    ++pos;
    record.deleted = getByte(buffer + pos) == 1;
    return record;
}

QList<Record> DatabaseManager::scanRecords(
    const std::function<bool(const Record &)> &predicate) const
{
    QList<Record> result;
    const qint64 length = m_dbFile.size();
    for (qint64 offset = 0; offset + Record::RECORD_SIZE <= length;
         offset += Record::RECORD_SIZE) {
        const Record record = readRecord(offset);
        if (!record.deleted && predicate(record)) {
            result.append(record);
        }
    }
    return result;
}

bool DatabaseManager::matchesField(const Record &record, const QString &field,
                                   const QString &value)
{
    if (field == QLatin1String("name")) {
        return record.name.compare(value, Qt::CaseInsensitive) == 0;
    }
    if (field == QLatin1String("category")) {
        return record.category.compare(value, Qt::CaseInsensitive) == 0;
    }
    if (field == QLatin1String("price")) {

        bool ok = false;
        const double parsed = value.trimmed().toDouble(&ok);
        return ok && qAbs(record.price - parsed) < 0.001;
    }
    if (field == QLatin1String("quantity")) {
        bool ok = false;
        const int parsed = value.toInt(&ok);
        return ok && record.quantity == parsed;
    }
    return false;
}

void DatabaseManager::copyFile(const QString &source, const QString &destination)
{
    if (!QFile::exists(source)) {
        throw DbException(QStringLiteral("File not found: %1").arg(source));
    }
    if (QFile::exists(destination) && !QFile::remove(destination)) {
        throw DbException(
            QStringLiteral("Cannot overwrite file: %1").arg(destination));
    }
    if (!QFile::copy(source, destination)) {
        throw DbException(QStringLiteral("Cannot copy %1 to %2").arg(source, destination));
    }
}

void DatabaseManager::ensureOpen() const
{
    if (!isOpen()) {
        throw DbException(QStringLiteral("No database is open"));
    }
}

}
