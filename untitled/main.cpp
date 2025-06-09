#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // для работы QSettings
    QCoreApplication::setOrganizationName("INEUM");\
    QCoreApplication::setApplicationName("SVNExplorer");

    MainWindow w;
    w.show();
    return a.exec();
}
