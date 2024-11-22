#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTime>

#include <qdltfile.h>
#include <qdltfilter.h>
#include <qdltimporter.h>
#include <qdltexporter.h>
#include <optmanager.h>

/*
 * Examples:
 *
 * -v
 * c:/_test/input1.mf4 c:/_test/output.dlt
 * c:/_test/input1.pcap output.dlt
 * c:/_test/input1.mf4 c:/_test/input2.mf4 c:/_test/output.dlt
 * c:/_test/input1.pcap c:/_test/input2.pcap c:/_test/output.dlt
 * -c c:/_test/output.txt c:/_test/input.txt
 * -csv -c c:/_test/output.csv c:/_test/input.dlt
 * -csv -c c:/_test/output.csv c:/_test/filter.dlf c:/_test/input.dlt
 * -d -c c:/_test/output.dlt c:/_test/filter.dlf c:/_test/input.dlt
 * -csv -c c:/_test/output.csv c:/_test/input1.mf4 c:/_test/input2.mf4 c:/_test/filter.dlf c:/_test/output.dlt
 *
 */

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
    if(opt.getLogFiles().size()<1)
    {
        qDebug() << "ERROR: No DLT file used. At least one DLT file must be provided.";
        return -1;
    }
    if(opt.getMf4Files().size()>0 || opt.getPcapFiles().size()>0)
    {
        if(opt.getLogFiles().size()>1)
        {
            qDebug() << "ERROR: When importing from MF4 or PCAP files only one DLT file is allowed.";
            return -1;
        }
        else
        {
            // open outputfile
            outputfile.setFileName(opt.getLogFiles()[0]);
            outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate);
            outputfile.close();
        }
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
            qDebug() << "Import MF4 File:" << i << "ms";
            QDltImporter importer(&outputfile);
            //QTime timeStart = QTime::currentTime();
            importer.dltIpcFromMF4(i);
            //qDebug() << "Duration:" << timeStart.msecsTo(QTime::currentTime());
        }
        // load PCAP files
        QStringList pcapFiles = opt.getPcapFiles();
        if(pcapFiles.size()>0)
            qDebug() << "### Load PCAP files";
        for ( const auto& i : pcapFiles )
        {
            qDebug() << "Import PCAP File:" << i;
            QDltImporter importer(&outputfile);
            importer.dltIpcFromPCAP(i);
        }
    }

    // Export
    if(!opt.getConvertDestFile().isEmpty())
    {       
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
            if(!filterList.LoadFilter(i,false))
                qDebug() << "ERROR: Failed loading filter:" << i;
            dltFile.setFilterList(filterList);
            dltFile.enableFilter(true);
        }

        // Create index
        qDebug() << "### Create index";
        dltFile.createIndex();
        qDebug() << "Number of messages:" << dltFile.size();

        // Create filter index
        //qDebug() << "### Create filter index";
        //dltFile.setFilterList(filterList);
        //dltFile.createIndexFilter();
        //qDebug() << "Number of messages matching filter:" << dltFile.sizeFilter();

        if(opt.get_convertionmode()==e_DLT)
        {
            qDebug() << "### Convert to DLT";
            QDltExporter exporter(&dltFile,opt.getConvertDestFile(),0,QDltExporter::FormatDlt,QDltExporter::SelectionAll,0,1,0,0,opt.getDelimiter());
            if(opt.isMultifilter())
                exporter.setMultifilterFilenames(opt.getFilterFiles());
            else
                exporter.setFilterList(filterList);
            qDebug() << "Commandline DLT convert to " << opt.getConvertDestFile();
            exporter.exportMessages();
            qDebug() << "DLT export to DLT file format done";
        }
        if(opt.get_convertionmode()==e_ASCI)
        {
            qDebug() << "### Convert to ASCII";
            QDltExporter exporter(&dltFile,opt.getConvertDestFile(),0,QDltExporter::FormatAscii,QDltExporter::SelectionAll,0,1,0,0,opt.getDelimiter());
            if(opt.isMultifilter())
                exporter.setMultifilterFilenames(opt.getFilterFiles());
            else
                exporter.setFilterList(filterList);
            qDebug() << "Commandline ASCII convert to " << opt.getConvertDestFile();
            exporter.exportMessages();
            qDebug() << "DLT export ASCII done";
        }
        if(opt.get_convertionmode()==e_CSV)
        {
            qDebug() << "### Convert to CSV";
            QDltExporter exporter(&dltFile,opt.getConvertDestFile(),0,QDltExporter::FormatCsv,QDltExporter::SelectionAll,0,1,0,0,opt.getDelimiter());
            if(opt.isMultifilter())
                exporter.setMultifilterFilenames(opt.getFilterFiles());
            else
                exporter.setFilterList(filterList);
            qDebug() << "Commandline CSV convert to " << opt.getConvertDestFile();
            exporter.exportMessages();
            qDebug() << "DLT export CSV done";
        }
        if(opt.get_convertionmode()==e_UTF8)
        {
            qDebug() << "### Convert to UTF8";
            QDltExporter exporter(&dltFile,opt.getConvertDestFile(),0,QDltExporter::FormatUTF8,QDltExporter::SelectionAll,0,1,0,0,opt.getDelimiter());
            if(opt.isMultifilter())
                exporter.setMultifilterFilenames(opt.getFilterFiles());
            else
                exporter.setFilterList(filterList);
            qDebug() << "Commandline UTF8 convert to " << opt.getConvertDestFile();
            exporter.exportMessages();
            qDebug() << "DLT export UTF8 done";
        }
    }

    // Terminate
    qDebug() << "### Terminate DLT Commander";

    //return a.exec();
    return 0;
}
