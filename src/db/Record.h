#pragma once

#include <QString>
#include <QStringList>

namespace db {

class Record {
public:
    static constexpr int RECORD_SIZE = 178;
    static constexpr int NAME_MAX_LEN = 50;
    static constexpr int CATEGORY_MAX_LEN = 30;

    int id = 0;
    QString name;
    double price = 0.0;
    int quantity = 0;
    QString category;
    bool inStock = false;
    bool deleted = false;

    Record() = default;
    Record(int id, const QString &name, double price, int quantity,
           const QString &category, bool inStock);

    QStringList toTableRow() const;
    static QStringList columnNames();
};

} // namespace db
