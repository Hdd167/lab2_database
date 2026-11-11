#include "DeleteDialog.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace gui {

namespace {

const QStringList &fieldLabels()
{
    static const QStringList labels{
        QStringLiteral("id (key)"),
        QStringLiteral("name"),
        QStringLiteral("category"),
        QStringLiteral("price"),
        QStringLiteral("quantity"),
    };
    return labels;
}

const QStringList &fieldKeys()
{
    static const QStringList keys{
        QStringLiteral("id"),
        QStringLiteral("name"),
        QStringLiteral("category"),
        QStringLiteral("price"),
        QStringLiteral("quantity"),
    };
    return keys;
}

} // namespace

DeleteDialog::DeleteDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Delete Record"));
    setModal(true);
    resize(360, 200);
    setMinimumWidth(340);

    m_fieldBox = new QComboBox(this);
    m_fieldBox->addItems(fieldLabels());
    m_valueEdit = new QLineEdit(this);

    auto *form = new QFormLayout;
    form->setContentsMargins(14, 14, 14, 8);
    form->setHorizontalSpacing(8);
    form->setVerticalSpacing(8);
    form->addRow(tr("Search by field:"), m_fieldBox);
    form->addRow(tr("Value:"), m_valueEdit);
    auto *note = new QLabel(tr("Deleting by non-key removes all matches."), this);
    note->setEnabled(false);
    form->addRow(QString(), note);

    auto *deleteButton = new QPushButton(tr("Delete"), this);
    auto *cancelButton = new QPushButton(tr("Cancel"), this);
    connect(deleteButton, &QPushButton::clicked, this, &DeleteDialog::onDelete);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(deleteButton);
    buttons->addWidget(cancelButton);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addStretch();
    layout->addLayout(buttons);
}

void DeleteDialog::onDelete()
{
    const int index = m_fieldBox->currentIndex();
    const QString text = m_valueEdit->text().trimmed();

    if (text.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Enter a value."));
        return;
    }

    if (index == 0) {
        bool ok = false;
        const int id = text.toInt(&ok);
        if (!ok) {
            QMessageBox::critical(this, tr("Error"), tr("ID must be an integer."));
            return;
        }
        m_keyValue = id;
        m_keyDelete = true;
    } else {
        m_keyDelete = false;
        m_field = fieldKeys().at(index);
        m_value = text;
    }

    m_confirmed = true;
    accept();
}

} // namespace gui
