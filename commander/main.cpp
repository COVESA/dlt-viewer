#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <qdltfile.h>
#include <qdltfilter.h>
#include <qdltimporter.h>
#include <qdltexporter.h>
#include <optmanager.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QDltFile dltFile;
    QDltFilterList filterList;
    OptManager opt;
    QFile outputfile;

    // Parse commandline parameters
    QStringList arguments = a.arguments();
    opt.parse(&arguments);

    // Perform some checks
    if(opt.getFilterFiles().size()<1)
    {
        qDebug() << "ERROR: No DLT file used. At least one DLT file must be provided.";
        return -1;
    }

    // Load dlt files
    qDebug() << "### Load DLT files";
    QStringList logFiles = opt.getLogFiles();
    for ( const auto& i : logFiles )
    {
        qDebug() << "Load DLT File:" << i;
        if(!dltFile.open(i))
            qDebug() << "ERROR: Failed loading file:" << i;
        outputfile.setFileName(i);
    }

    // Load filter
    qDebug() << "### Load filters";
    QStringList filterFiles = opt.getFilterFiles();
    for ( const auto& i : filterFiles )
    {
        qDebug() << "Load DLT Filter:" << i;
        if(!filterList.LoadFilter(i,true))
            qDebug() << "ERROR: Failed loading filter:" << i;
        dltFile.setFilterList(filterList);
        dltFile.enableFilter(true);
    }

    // Import
    if(!outputfile.fileName().isEmpty())
    {
        // load MF4 files
        QStringList mf4Files = opt.getMf4Files();
        if(mf4Files.size()>0)
            qDebug() << "### Load MF4 files";
        for ( const auto& i : mf4Files )
        {
            qDebug() << "Import MF4 File:" << i;
            QDltImporter importer;
            importer.dltIpcFromMF4(outputfile,i,0,false);
        }
        // load PCAP files
        QStringList pcapFiles = opt.getPcapFiles();
        if(pcapFiles.size()>0)
            qDebug() << "### Load PCAP files";
        for ( const auto& i : pcapFiles )
        {
            qDebug() << "Import PCAP File:" << i;
            QDltImporter importer;
            importer.dltIpcFromPCAP(outputfile,i,0,false);
        }
    }

    // Create index
    qDebug() << "### Create index";
    dltFile.createIndex();
    qDebug() << "Number of messages:" << dltFile.size();

    // Create filter index
    qDebug() << "### Create filter index";
    dltFile.setFilterList(filterList);
    dltFile.createIndexFilter();
    qDebug() << "Number of messages matching filter:" << dltFile.sizeFilter();

    // Export
    if(!opt.getConvertDestFile().isEmpty())
    {
        if(opt.get_convertionmode()==e_DLT)
        {
            QFile output(opt.getConvertDestFile());
            qDebug() << "### Convert to DLT";
            QDltExporter exporter(0,0,0);
            qDebug() << "Commandline DLT convert to " << opt.getConvertDestFile();
            exporter.exportMessages(&dltFile,&output,0,QDltExporter::FormatDlt,QDltExporter::SelectionFiltered);
            qDebug() << "DLT export to DLT file format done";
        }
        if(opt.get_convertionmode()==e_ASCI)
        {
            QFile output(opt.getConvertDestFile());
            qDebug() << "### Convert to ASCII";
            QDltExporter exporter(0,0,0);
            qDebug() << "Commandline ASCII convert to " << opt.getConvertDestFile();
            exporter.exportMessages(&dltFile,&output,0,QDltExporter::FormatAscii,QDltExporter::SelectionFiltered);
            qDebug() << "DLT export ASCII done";
        }
        if(opt.get_convertionmode()==e_CSV)
        {
            QFile output(opt.getConvertDestFile());
            qDebug() << "### Convert to CSV";
            QDltExporter exporter(0,0,0);
            qDebug() << "Commandline ASCII convert to " << opt.getConvertDestFile();
            exporter.exportMessages(&dltFile,&output,0,QDltExporter::FormatCsv,QDltExporter::SelectionFiltered);
            qDebug() << "DLT export CSV done";
        }
        if(opt.get_convertionmode()==e_UTF8)
        {
            QFile output(opt.getConvertDestFile());
            qDebug() << "### Convert to UTF8";
            QDltExporter exporter(0,0,0);
            qDebug() << "Commandline UTF8 convert to " << opt.getConvertDestFile();
            exporter.exportMessages(&dltFile,&output,0,QDltExporter::FormatUTF8,QDltExporter::SelectionFiltered);
            qDebug() << "DLT export UTF8 done";
        }
    }
    else
        qDebug() << "ERROR: Export not possible, no DLT file provided";

    // Terminate
    qDebug() << "### Terminate DLT Commander";

    //return a.exec();
    return 0;
}
