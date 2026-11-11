#include "gui/MainFrame.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Product Database"));

    gui::MainFrame frame;
    frame.show();

    return application.exec();
}
