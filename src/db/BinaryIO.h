#pragma once

#include <QChar>
#include <QString>
#include <QtGlobal>
#include <cstring>

namespace db {

// The files produced here are byte compatible with the Java version: every
// number is big endian and characters are stored as UTF-16 code units, exactly
// like java.io.DataOutput does.

inline void putByte(char *dst, quint8 value)
{
    dst[0] = char(value);
}

inline void putChar(char *dst, char16_t value)
{
    dst[0] = char((value >> 8) & 0xFF);
    dst[1] = char(value & 0xFF);
}

inline void putInt(char *dst, qint32 value)
{
    const quint32 u = quint32(value);
    dst[0] = char((u >> 24) & 0xFF);
    dst[1] = char((u >> 16) & 0xFF);
    dst[2] = char((u >> 8) & 0xFF);
    dst[3] = char(u & 0xFF);
}

inline void putLong(char *dst, qint64 value)
{
    const quint64 u = quint64(value);
    putInt(dst, qint32(u >> 32));
    putInt(dst + 4, qint32(u & 0xFFFFFFFFu));
}

inline void putDouble(char *dst, double value)
{
    quint64 bits = 0;
    static_assert(sizeof(bits) == sizeof(value), "double must be 64 bit");
    std::memcpy(&bits, &value, sizeof(bits));
    putLong(dst, qint64(bits));
}

inline quint8 getByte(const char *src)
{
    return quint8(src[0]);
}

inline char16_t getChar(const char *src)
{
    return char16_t((quint16(quint8(src[0])) << 8) | quint16(quint8(src[1])));
}

inline qint32 getInt(const char *src)
{
    const quint32 u = (quint32(quint8(src[0])) << 24) | (quint32(quint8(src[1])) << 16) |
                      (quint32(quint8(src[2])) << 8) | quint32(quint8(src[3]));
    return qint32(u);
}

inline qint64 getLong(const char *src)
{
    const quint64 u = (quint64(quint32(getInt(src))) << 32) | quint64(quint32(getInt(src + 4)));
    return qint64(u);
}

inline double getDouble(const char *src)
{
    const quint64 bits = quint64(getLong(src));
    double value = 0.0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

// Fixed size, NUL padded UTF-16 string (Java's DataOutput.writeChars).
inline void writeString(char *dst, const QString &text, int maxChars)
{
    for (int i = 0; i < maxChars; ++i) {
        const char16_t unit = i < text.size() ? text.at(i).unicode() : char16_t(0);
        putChar(dst + 2 * i, unit);
    }
}

inline QString readString(const char *src, int maxChars)
{
    QString result;
    result.reserve(maxChars);
    for (int i = 0; i < maxChars; ++i) {
        const char16_t unit = getChar(src + 2 * i);
        if (unit != 0) {
            result.append(QChar(unit));
        }
    }
    return result;
}

} // namespace db
