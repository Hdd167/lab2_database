#include "RecordDialog.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <charconv>

namespace gui {

namespace {

// Shortest text that reads back as the same double, like Java's Double.toString.
// QString::number() would round to six significant digits and lose the value.
QString numberText(double value)
{
    char buffer[64];
    const std::to_chars_result converted = std::to_chars(buffer, buffer + sizeof(buffer), value);
    if (converted.ec != std::errc()) {
        return QString::number(value);
    }
    return QString::fromLatin1(buffer, int(converted.ptr - buffer));
}

} // namespace

RecordDialog::RecordDialog(QWidget *parent, const QString &title,
                           const std::optional<db::Record> &existing)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    resize(420, 300);
    setMinimumWidth(360);

    const bool editing = existing.has_value();

    m_idEdit = new QLineEdit(this);
    m_nameEdit = new QLineEdit(this);
    m_priceEdit = new QLineEdit(this);
    m_quantityEdit = new QLineEdit(this);
    m_categoryEdit = new QLineEdit(this);
    m_inStockBox = new QCheckBox(this);

    if (editing) {
        const db::Record &record = *existing;
        m_idEdit->setText(QString::number(record.id));
        m_nameEdit->setText(record.name);
        m_priceEdit->setText(numberText(record.price));
        m_quantityEdit->setText(QString::number(record.quantity));
        m_categoryEdit->setText(record.category);
        m_inStockBox->setChecked(record.inStock);
    }
    m_idEdit->setReadOnly(editing);

    auto *form = new QFormLayout;
    form->setContentsMargins(14, 14, 14, 8);
    form->setHorizontalSpacing(8);
    form->setVerticalSpacing(8);
    form->addRow(tr("ID (key):"), m_idEdit);
    form->addRow(tr("Name:"), m_nameEdit);
    form->addRow(tr("Price:"), m_priceEdit);
    form->addRow(tr("Quantity:"), m_quantityEdit);
    form->addRow(tr("Category:"), m_categoryEdit);
    form->addRow(tr("In Stock:"), m_inStockBox);

    auto *okButton = new QPushButton(tr("OK"), this);
    auto *cancelButton = new QPushButton(tr("Cancel"), this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &RecordDialog::onOk);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(okButton);
    buttons->addWidget(cancelButton);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addStretch();
    layout->addLayout(buttons);
}

void RecordDialog::onOk()
{
    bool idOk = false;
    bool priceOk = false;
    bool quantityOk = false;

    const int id = m_idEdit->text().trimmed().toInt(&idOk);
    const QString name = m_nameEdit->text().trimmed();
    const double price = m_priceEdit->text().trimmed().toDouble(&priceOk);
    const int quantity = m_quantityEdit->text().trimmed().toInt(&quantityOk);
    const QString category = m_categoryEdit->text().trimmed();
    const bool inStock = m_inStockBox->isChecked();

    if (!idOk || !priceOk || !quantityOk) {
        QMessageBox::critical(this, tr("Input Error"),
                              tr("ID, Price, and Quantity must be valid numbers."));
        return;
    }

    QString error;
    if (name.isEmpty()) {
        error = tr("Name cannot be empty.");
    } else if (price < 0) {
        error = tr("Price must be >= 0.");
    } else if (quantity < 0) {
        error = tr("Quantity must be >= 0.");
    } else if (name.size() > db::Record::NAME_MAX_LEN) {
        error = tr("Name too long (max %1 chars).").arg(db::Record::NAME_MAX_LEN);
    } else if (category.size() > db::Record::CATEGORY_MAX_LEN) {
        error = tr("Category too long (max %1 chars).").arg(db::Record::CATEGORY_MAX_LEN);
    }
    if (!error.isEmpty()) {
        QMessageBox::critical(this, tr("Input Error"), error);
        return;
    }

    m_result = db::Record(id, name, price, quantity, category, inStock);
    accept();
}

} // namespace gui
