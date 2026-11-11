#include "Record.h"

namespace db {

Record::Record(int id, const QString &name, double price, int quantity,
               const QString &category, bool inStock)
    : id(id), name(name), price(price), quantity(quantity),
      category(category), inStock(inStock), deleted(false)
{
}

QStringList Record::toTableRow() const
{
    return QStringList{
        QString::number(id),
        name,
        QString::number(price, 'f', 2),
        QString::number(quantity),
        category,
        inStock ? QStringLiteral("true") : QStringLiteral("false"),
    };
}

QStringList Record::columnNames()
{
    return QStringList{
        QStringLiteral("ID"),
        QStringLiteral("Name"),
        QStringLiteral("Price"),
        QStringLiteral("Quantity"),
        QStringLiteral("Category"),
        QStringLiteral("In Stock"),
    };
}

} // namespace db
