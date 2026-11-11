#pragma once

#include "db/DatabaseManager.h"

#include <QMainWindow>

#include <functional>

class QAction;
class QMenu;
class QTableWidget;
class QToolBar;

namespace gui {

// Main window: menu bar, tool bar, record table and status bar.
class MainFrame : public QMainWindow {
    Q_OBJECT

public:
    explicit MainFrame(QWidget *parent = nullptr);

private:
    void newDatabase();
    void openDatabase();
    void save();
    void clearDatabase();
    void deleteDatabase();
    void addRecord();
    void editRecord();
    void deleteRecord();
    void search();
    void createBackup();
    void restoreBackup();
    void refreshTable();

    void buildMenuBar();
    QToolBar *buildToolBar();
    QAction *makeAction(const QString &text, const std::function<void()> &handler);
    QString askSavePath(const QString &title);
    QString askOpenPath(const QString &title);
    bool requireOpen();
    static QString stripExt(const QString &path);
    void setStatus(const QString &message);
    void showError(const QString &message);
    bool confirm(const QString &message);

    db::DatabaseManager m_db;
    QTableWidget *m_table = nullptr;
};

} // namespace gui
