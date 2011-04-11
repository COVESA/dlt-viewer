#include <QtGui/QApplication>
#include <QModelIndex>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString filename;
    if(argc==2)
        filename = QString(argv[1]);

    MainWindow w(filename);
    w.show();

    return a.exec();
}
