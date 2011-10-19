#include <QtGui/QApplication>
#include <QModelIndex>

#include "mainwindow.h"
#include "optmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList arguments = a.arguments();
    OptManager *opt = OptManager::getInstance();
    opt->parse(&arguments);

    MainWindow w;
    w.show();

    return a.exec();
}
