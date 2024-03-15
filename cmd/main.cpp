#include <QCoreApplication>
#include <QDebug>

#include <qdltfile.h>
#include <qdltfilter.h>
#include <optmanager.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "*** Start";

    QDltFile dltFile;
    QDltFilterList filterList;
    OptManager opt;

    // parse commandline parameters
    qDebug() << "*** Parse Arguments";
    QStringList arguments = a.arguments();
    opt.parse(&arguments);

    // load filter
    qDebug() << "*** Load DLT Filter";
    QString filterFile = opt.getFilterFile();
    if(!filterFile.isEmpty())
    {
        qDebug() << "Load DLT Filter:" << filterFile;
        if(!filterList.LoadFilter(filterFile,true))
            qDebug() << "ERROR: Failed loading filter:" << filterFile;
    }

    // load dlt files
    qDebug() << "*** Load DLT Files";
    QStringList logFiles = opt.getLogFiles();
    for ( const auto& i : logFiles )
    {
        qDebug() << "Load DLT File:" << i;
        if(!dltFile.open(i))
            qDebug() << "ERROR: Failed loading file:" << i;
    }

    // create index
    qDebug() << "*** Create Index";
    dltFile.createIndex();
    qDebug() << "Number of messages:" << dltFile.size();

    // create filterindex
    qDebug() << "*** Create Filter Index";
    dltFile.setFilterList(filterList);
    dltFile.createIndexFilter();
    qDebug() << "Number of messages matching filter:" << dltFile.sizeFilter();

    qDebug() << "*** Finish";

    return a.exec();
}
