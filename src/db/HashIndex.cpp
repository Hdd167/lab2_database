#include "HashIndex.h"

#include "BinaryIO.h"
#include "DbException.h"

#include <QByteArray>

namespace db {

HashIndex::HashIndex(const QString &path, bool create)
{
    m_file.setFileName(path);
    if (!m_file.open(QIODevice::ReadWrite)) {
        throw DbException(QStringLiteral("Cannot open index file %1: %2")
                              .arg(path, m_file.errorString()));
    }
    m_open = true;

    if (create) {
        if (!m_file.resize(0)) {
            throw DbException(QStringLiteral("Cannot create index file %1: %2")
                                  .arg(path, m_file.errorString()));
        }
        writeEmptyTable();
    }
}

HashIndex::~HashIndex()
{
    try {
        close();
    } catch (const DbException &) {
    }
}

void HashIndex::writeEmptyTable()
{
    char header[HEADER_SIZE];
    putInt(header, CAPACITY);
    QByteArray empty(CAPACITY * SLOT_SIZE, '\0');

    if (!m_file.seek(0) || m_file.write(header, HEADER_SIZE) != HEADER_SIZE
        || m_file.write(empty) != empty.size() || !m_file.flush()) {
        throw DbException(
            QStringLiteral("Cannot write hash index: %1").arg(m_file.errorString()));
    }
}

void HashIndex::readSlot(int slot, quint8 &status, int &key, qint64 &offset) const
{
    char buffer[SLOT_SIZE];
    if (!m_file.seek(slotPos(slot)) || m_file.read(buffer, SLOT_SIZE) != SLOT_SIZE) {
        throw DbException(
            QStringLiteral("Hash index is damaged or truncated: %1").arg(m_file.fileName()));
    }
    status = getByte(buffer);
    key = getInt(buffer + 1);
    offset = getLong(buffer + 5);
}

void HashIndex::writeSlot(int slot, int key, qint64 offset)
{
    char buffer[SLOT_SIZE];
    putByte(buffer, STATUS_OCCUPIED);
    putInt(buffer + 1, key);
    putLong(buffer + 5, offset);

    if (!m_file.seek(slotPos(slot)) || m_file.write(buffer, SLOT_SIZE) != SLOT_SIZE
        || !m_file.flush()) {
        throw DbException(
            QStringLiteral("Cannot write hash index: %1").arg(m_file.errorString()));
    }
}

qint64 HashIndex::find(int key) const
{
    const int h = hash(key);
    for (int i = 0; i < CAPACITY; ++i) {
        const int slot = (h + i) % CAPACITY;
        quint8 status = STATUS_EMPTY;
        int slotKey = 0;
        qint64 offset = 0;
        readSlot(slot, status, slotKey, offset);

        if (status == STATUS_EMPTY) {
            return -1;
        }
        if (status == STATUS_OCCUPIED && slotKey == key) {
            return offset;
        }
    }
    return -1;
}

void HashIndex::insert(int key, qint64 offset)
{
    const int h = hash(key);
    int tombstoneSlot = -1;

    for (int i = 0; i < CAPACITY; ++i) {
        const int slot = (h + i) % CAPACITY;
        quint8 status = STATUS_EMPTY;
        int slotKey = 0;
        qint64 slotOffset = 0;
        readSlot(slot, status, slotKey, slotOffset);

        if (status == STATUS_EMPTY) {
            writeSlot(tombstoneSlot >= 0 ? tombstoneSlot : slot, key, offset);
            return;
        }
        if (status == STATUS_TOMBSTONE) {
            if (tombstoneSlot < 0) {
                tombstoneSlot = slot;
            }
        } else if (slotKey == key) {
            throw DuplicateKeyException(key);
        }
    }

    if (tombstoneSlot >= 0) {
        writeSlot(tombstoneSlot, key, offset);
        return;
    }
    throw DbException(QStringLiteral("Hash index is full"));
}

bool HashIndex::remove(int key)
{
    const int h = hash(key);
    for (int i = 0; i < CAPACITY; ++i) {
        const int slot = (h + i) % CAPACITY;
        quint8 status = STATUS_EMPTY;
        int slotKey = 0;
        qint64 slotOffset = 0;
        readSlot(slot, status, slotKey, slotOffset);

        if (status == STATUS_EMPTY) {
            return false;
        }
        if (status == STATUS_OCCUPIED && slotKey == key) {
            char buffer[SLOT_SIZE];
            putByte(buffer, STATUS_TOMBSTONE);
            if (!m_file.seek(slotPos(slot)) || m_file.write(buffer, 1) != 1 || !m_file.flush()) {
                throw DbException(
                    QStringLiteral("Cannot write hash index: %1").arg(m_file.errorString()));
            }
            return true;
        }
    }
    return false;
}

void HashIndex::clear()
{
    writeEmptyTable();
}

void HashIndex::close()
{
    if (m_open) {
        m_file.close();
        m_open = false;
    }
}

} // namespace db
