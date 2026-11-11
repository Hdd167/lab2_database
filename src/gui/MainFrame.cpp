#include "MainFrame.h"

#include "DeleteDialog.h"
#include "RecordDialog.h"
#include "SearchDialog.h"

#include "db/DbException.h"
#include "db/Record.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QToolBar>

#include <optional>

namespace gui {

MainFrame::MainFrame(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Product Database"));
    resize(960, 580);

    m_table = new QTableWidget(0, db::Record::columnNames().size(), this);
    m_table->setHorizontalHeaderLabels(db::Record::columnNames());
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    setCentralWidget(m_table);

    buildMenuBar();
    addToolBar(buildToolBar());
    statusBar()->showMessage(tr("Ready"));
}

void MainFrame::buildMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(makeAction(tr("New Database..."), [this] { newDatabase(); }));
    fileMenu->addAction(makeAction(tr("Open Database..."), [this] { openDatabase(); }));
    fileMenu->addSeparator();
    fileMenu->addAction(makeAction(tr("Save"), [this] { save(); }));
    fileMenu->addAction(makeAction(tr("Clear Database"), [this] { clearDatabase(); }));
    fileMenu->addAction(makeAction(tr("Delete Database"), [this] { deleteDatabase(); }));
    fileMenu->addSeparator();
    fileMenu->addAction(makeAction(tr("Exit"), [] { QApplication::quit(); }));

    QMenu *backupMenu = menuBar()->addMenu(tr("Backup"));
    backupMenu->addAction(makeAction(tr("Create Backup..."), [this] { createBackup(); }));
    backupMenu->addAction(
        makeAction(tr("Restore from Backup..."), [this] { restoreBackup(); }));
}

QToolBar *MainFrame::buildToolBar()
{
    QToolBar *toolBar = addToolBar(tr("Main"));
    toolBar->setMovable(false);
    toolBar->addAction(makeAction(tr("Add"), [this] { addRecord(); }));
    toolBar->addAction(makeAction(tr("Edit"), [this] { editRecord(); }));
    toolBar->addAction(makeAction(tr("Delete"), [this] { deleteRecord(); }));
    toolBar->addSeparator();
    toolBar->addAction(makeAction(tr("Search"), [this] { search(); }));
    toolBar->addSeparator();
    toolBar->addAction(makeAction(tr("Refresh"), [this] { refreshTable(); }));
    return toolBar;
}

QAction *MainFrame::makeAction(const QString &text, const std::function<void()> &handler)
{
    auto *action = new QAction(text, this);
    connect(action, &QAction::triggered, this, [handler] { handler(); });
    return action;
}

QString MainFrame::askSavePath(const QString &title)
{
    return QFileDialog::getSaveFileName(this, title, QDir::currentPath(),
                                        tr("Database files (*.dat)"));
}

QString MainFrame::askOpenPath(const QString &title)
{
    return QFileDialog::getOpenFileName(this, title, QDir::currentPath(),
                                        tr("Database files (*.dat)"));
}

void MainFrame::newDatabase()
{
    const QString chosen = askSavePath(tr("Create New Database"));
    if (chosen.isEmpty()) {
        return;
    }
    try {
        const QString path = stripExt(chosen);
        m_db.create(path);
        refreshTable();
        setStatus(tr("Created: %1").arg(path));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::openDatabase()
{
    const QString chosen = askOpenPath(tr("Open Database"));
    if (chosen.isEmpty()) {
        return;
    }
    try {
        const QString path = stripExt(chosen);
        m_db.open(path);
        refreshTable();
        setStatus(tr("Opened: %1").arg(path));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::save()
{
    if (!requireOpen()) {
        return;
    }
    setStatus(tr("All changes are saved automatically."));
}

void MainFrame::clearDatabase()
{
    if (!requireOpen()) {
        return;
    }
    if (!confirm(tr("Clear all records from the database?"))) {
        return;
    }
    try {
        m_db.clear();
        refreshTable();
        setStatus(tr("Database cleared."));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::deleteDatabase()
{
    if (!requireOpen()) {
        return;
    }
    if (!confirm(tr("Permanently delete the database files? This cannot be undone."))) {
        return;
    }
    try {
        m_db.deleteDatabase();
        m_table->setRowCount(0);
        setStatus(tr("Database deleted."));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::addRecord()
{
    if (!requireOpen()) {
        return;
    }
    RecordDialog dialog(this, tr("Add Record"), std::nullopt);
    if (dialog.exec() != QDialog::Accepted || !dialog.result()) {
        return;
    }
    const db::Record record = *dialog.result();
    try {
        m_db.addRecord(record);
        refreshTable();
        setStatus(tr("Added record ID=%1").arg(record.id));
    } catch (const db::DuplicateKeyException &) {
        showError(tr("A record with ID=%1 already exists.").arg(record.id));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::editRecord()
{
    if (!requireOpen()) {
        return;
    }
    const int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0)) {
        showError(tr("Select a record in the table to edit."));
        return;
    }
    const int id = m_table->item(row, 0)->text().toInt();
    try {
        const std::optional<db::Record> existing = m_db.findByKey(id);
        if (!existing) {
            showError(tr("Record not found."));
            return;
        }
        RecordDialog dialog(this, tr("Edit Record"), existing);
        if (dialog.exec() != QDialog::Accepted || !dialog.result()) {
            return;
        }
        m_db.editRecord(id, *dialog.result());
        refreshTable();
        setStatus(tr("Updated record ID=%1").arg(id));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::deleteRecord()
{
    if (!requireOpen()) {
        return;
    }
    DeleteDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted || !dialog.isConfirmed()) {
        return;
    }
    try {
        if (dialog.isKeyDelete()) {
            if (m_db.deleteByKey(dialog.keyValue())) {
                setStatus(tr("Deleted record ID=%1").arg(dialog.keyValue()));
            } else {
                showError(tr("Record with ID=%1 not found.").arg(dialog.keyValue()));
            }
        } else {
            const int count = m_db.deleteByField(dialog.field(), dialog.value());
            setStatus(tr("Deleted %1 record(s) where %2=%3")
                          .arg(count)
                          .arg(dialog.field(), dialog.value()));
        }
        refreshTable();
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::search()
{
    if (!requireOpen()) {
        return;
    }
    auto *dialog = new SearchDialog(this, m_db);
    dialog->show();
}

void MainFrame::createBackup()
{
    if (!requireOpen()) {
        return;
    }
    const QString chosen = askSavePath(tr("Save Backup As"));
    if (chosen.isEmpty()) {
        return;
    }
    try {
        const QString path = stripExt(chosen);
        m_db.backup(path);
        setStatus(tr("Backup saved to: %1").arg(path));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::restoreBackup()
{
    if (!m_db.isOpen()) {
        showError(tr("Open a database first, then restore a backup into it."));
        return;
    }
    const QString chosen = askOpenPath(tr("Select Backup (.dat file)"));
    if (chosen.isEmpty()) {
        return;
    }
    if (!confirm(tr("This will overwrite the current database. Continue?"))) {
        return;
    }
    try {
        const QString path = stripExt(chosen);
        m_db.restore(path);
        refreshTable();
        setStatus(tr("Restored from backup: %1").arg(path));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

void MainFrame::refreshTable()
{
    m_table->setRowCount(0);
    if (!m_db.isOpen()) {
        return;
    }
    try {
        const QList<db::Record> records = m_db.getAllRecords();
        m_table->setRowCount(records.size());
        for (int row = 0; row < records.size(); ++row) {
            const QStringList values = records.at(row).toTableRow();
            for (int column = 0; column < values.size(); ++column) {
                m_table->setItem(row, column, new QTableWidgetItem(values.at(column)));
            }
        }
        setStatus(tr("Loaded %1 record(s).  DB: %2").arg(records.size()).arg(m_db.basePath()));
    } catch (const db::DbException &e) {
        showError(e.message());
    }
}

bool MainFrame::requireOpen()
{
    if (m_db.isOpen()) {
        return true;
    }
    showError(tr("No database is open."));
    return false;
}

QString MainFrame::stripExt(const QString &path)
{
    if (path.endsWith(QLatin1String(".dat")) || path.endsWith(QLatin1String(".idx"))) {
        return path.left(path.size() - 4);
    }
    return path;
}

void MainFrame::setStatus(const QString &message)
{
    statusBar()->showMessage(message);
}

void MainFrame::showError(const QString &message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

bool MainFrame::confirm(const QString &message)
{
    return QMessageBox::question(this, tr("Confirm"), message,
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No)
           == QMessageBox::Yes;
}

} // namespace gui
