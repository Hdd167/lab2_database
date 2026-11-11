#pragma once

#include <QFile>
#include <QString>
#include <QtGlobal>

namespace db {

// Disk based hash index, laid out exactly like the Java version:
// 4 byte capacity header followed by CAPACITY slots of 13 bytes each
// (1 byte status + 4 byte key + 8 byte record offset).
class HashIndex {
public:
    static constexpr int CAPACITY = 10007;

    HashIndex(const QString &path, bool create);
    ~HashIndex();

    HashIndex(const HashIndex &) = delete;
    HashIndex &operator=(const HashIndex &) = delete;

    qint64 find(int key) const;
    void insert(int key, qint64 offset);
    bool remove(int key);
    void clear();
    void close();

private:
    static constexpr int HEADER_SIZE = 4;
    static constexpr int SLOT_SIZE = 13;
    static constexpr quint8 STATUS_EMPTY = 0;
    static constexpr quint8 STATUS_OCCUPIED = 1;
    static constexpr quint8 STATUS_TOMBSTONE = 2;

    static qint64 slotPos(int slot) { return HEADER_SIZE + qint64(slot) * SLOT_SIZE; }
    static int hash(int key) { return (key & 0x7FFFFFFF) % CAPACITY; }

    void writeEmptyTable();
    void readSlot(int slot, quint8 &status, int &key, qint64 &offset) const;
    void writeSlot(int slot, int key, qint64 offset);

    mutable QFile m_file;
    bool m_open = false;
};

} // namespace db
