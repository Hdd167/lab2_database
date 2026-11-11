#pragma once

#include "HashIndex.h"
#include "Record.h"

#include <QFile>
#include <QList>
#include <QString>
#include <QtGlobal>

#include <functional>
#include <memory>
#include <optional>

namespace db {

// Stores fixed size records in "<base>.dat" and keeps their key to offset
// mapping in "<base>.idx" (see HashIndex).
class DatabaseManager {
public:
    DatabaseManager() = default;
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    void create(const QString &basePath);
    void open(const QString &basePath);
    void close();
    void clear();
    void deleteDatabase();

    void addRecord(const Record &record);
    std::optional<Record> findByKey(int id) const;
    QList<Record> findByField(const QString &field, const QString &value) const;
    bool deleteByKey(int id);
    int deleteByField(const QString &field, const QString &value);
    bool editRecord(int id, const Record &newData);
    QList<Record> getAllRecords() const;

    void backup(const QString &backupBase) const;
    void restore(const QString &backupBase);

    bool isOpen() const { return m_dbFile.isOpen(); }
    QString basePath() const { return m_basePath; }

private:
    void ensureOpen() const;
    void openFiles(const QString &basePath, bool create);
    void writeRecord(const Record &record, qint64 offset);
    Record readRecord(qint64 offset) const;
    QList<Record> scanRecords(const std::function<bool(const Record &)> &predicate) const;
    static bool matchesField(const Record &record, const QString &field, const QString &value);
    static void copyFile(const QString &source, const QString &destination);

    // Mutable because reading jumps the file pointer around; the database
    // contents themselves are not modified by the const methods.
    mutable QFile m_dbFile;
    std::unique_ptr<HashIndex> m_index;
    QString m_basePath;
};

} // namespace db
