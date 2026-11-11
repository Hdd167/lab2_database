#pragma once

#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QTableWidget;

namespace db {
class DatabaseManager;
}

namespace gui {

// Modeless dialog searching the open database either by key or by field.
class SearchDialog : public QDialog {
    Q_OBJECT

public:
    SearchDialog(QWidget *parent, db::DatabaseManager &database);

private:
    void runSearch();

    db::DatabaseManager &m_database;
    QComboBox *m_fieldBox = nullptr;
    QLineEdit *m_valueEdit = nullptr;
    QTableWidget *m_resultTable = nullptr;
    QLabel *m_countLabel = nullptr;
};

} // namespace gui
