#pragma once

#include "db/Record.h"

#include <QDialog>

#include <optional>

class QCheckBox;
class QLineEdit;

namespace gui {

// Modal dialog used both for adding and for editing a record. When
// "existing" is given the record is edited and the key cannot be changed.
class RecordDialog : public QDialog {
    Q_OBJECT

public:
    RecordDialog(QWidget *parent, const QString &title,
                 const std::optional<db::Record> &existing);

    std::optional<db::Record> result() const { return m_result; }

private:
    void onOk();

    QLineEdit *m_idEdit = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QLineEdit *m_priceEdit = nullptr;
    QLineEdit *m_quantityEdit = nullptr;
    QLineEdit *m_categoryEdit = nullptr;
    QCheckBox *m_inStockBox = nullptr;
    std::optional<db::Record> m_result;
};

} // namespace gui
