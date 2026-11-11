#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QLineEdit;

namespace gui {

// Modal dialog asking which field to delete by: the key deletes a single
// record, any other field deletes every match.
class DeleteDialog : public QDialog {
    Q_OBJECT

public:
    explicit DeleteDialog(QWidget *parent);

    bool isConfirmed() const { return m_confirmed; }
    bool isKeyDelete() const { return m_keyDelete; }
    int keyValue() const { return m_keyValue; }
    QString field() const { return m_field; }
    QString value() const { return m_value; }

private:
    void onDelete();

    QComboBox *m_fieldBox = nullptr;
    QLineEdit *m_valueEdit = nullptr;

    bool m_confirmed = false;
    bool m_keyDelete = false;
    int m_keyValue = 0;
    QString m_field;
    QString m_value;
};

} // namespace gui
