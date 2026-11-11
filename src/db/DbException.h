#pragma once

#include <QString>
#include <stdexcept>

namespace db {

// Equivalent of Java's IOException.
class DbException : public std::runtime_error {
public:
    explicit DbException(const QString &message)
        : std::runtime_error(message.toUtf8().toStdString()), m_message(message) {}

    QString message() const { return m_message; }

private:
    QString m_message;
};

// Equivalent of Java's IllegalArgumentException thrown by HashIndex::insert.
class DuplicateKeyException : public DbException {
public:
    explicit DuplicateKeyException(int key)
        : DbException(QStringLiteral("Duplicate key: %1").arg(key)), m_key(key) {}

    int key() const { return m_key; }

private:
    int m_key;
};

} // namespace db
