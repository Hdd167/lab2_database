#include "SearchDialog.h"

#include "db/DatabaseManager.h"
#include "db/DbException.h"
#include "db/Record.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <optional>

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

SearchDialog::SearchDialog(QWidget *parent, db::DatabaseManager &database)
    : QDialog(parent), m_database(database)
{
    setWindowTitle(tr("Search"));
    setModal(false);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(750, 420);

    m_fieldBox = new QComboBox(this);
    m_fieldBox->addItems(fieldLabels());
    m_valueEdit = new QLineEdit(this);
    auto *searchButton = new QPushButton(tr("Search"), this);
    m_countLabel = new QLabel(tr("Results: —"), this);

    auto *top = new QHBoxLayout;
    top->addWidget(new QLabel(tr("Field:"), this));
    top->addWidget(m_fieldBox);
    top->addWidget(new QLabel(tr("Value:"), this));
    top->addWidget(m_valueEdit, 1);
    top->addWidget(searchButton);
    top->addStretch();

    m_resultTable = new QTableWidget(0, db::Record::columnNames().size(), this);
    m_resultTable->setHorizontalHeaderLabels(db::Record::columnNames());
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_resultTable->verticalHeader()->setVisible(false);

    auto *bottom = new QHBoxLayout;
    bottom->addWidget(m_countLabel);
    bottom->addStretch();

    connect(searchButton, &QPushButton::clicked, this, &SearchDialog::runSearch);
    connect(m_valueEdit, &QLineEdit::returnPressed, this, &SearchDialog::runSearch);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addLayout(top);
    layout->addWidget(m_resultTable);
    layout->addLayout(bottom);
}

void SearchDialog::runSearch()
{
    const QString text = m_valueEdit->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    const int index = m_fieldBox->currentIndex();
    m_resultTable->setRowCount(0);

    try {
        QList<db::Record> results;
        if (index == 0) {
            bool ok = false;
            const int id = text.toInt(&ok);
            if (!ok) {
                QMessageBox::critical(this, tr("Error"), tr("ID must be an integer."));
                return;
            }
            if (const std::optional<db::Record> record = m_database.findByKey(id)) {
                results.append(*record);
            }
        } else {
            results = m_database.findByField(fieldKeys().at(index), text);
        }

        m_resultTable->setRowCount(results.size());
        for (int row = 0; row < results.size(); ++row) {
            const QStringList values = results.at(row).toTableRow();
            for (int column = 0; column < values.size(); ++column) {
                m_resultTable->setItem(row, column, new QTableWidgetItem(values.at(column)));
            }
        }
        m_countLabel->setText(tr("Results: %1").arg(results.size()));
    } catch (const db::DbException &e) {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

} // namespace gui
