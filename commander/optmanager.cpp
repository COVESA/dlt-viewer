/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file optmanager.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#if (WIN32)
#include <windows.h>
#endif

#include "optmanager.h"
#include "../src/version.h"
#include "qdltexporter.h"

#include <QDebug>
#include <QFileInfo>
#include <iostream>

namespace {
Split parseSplitOption(const QString& splitOption) {
    Split result;
    result.size = splitOption.left(splitOption.length() - 1).toULong();
    if (splitOption.endsWith("K")) {
        result.unit = Units::KB;
    } else if (splitOption.endsWith("M")) {
        result.unit = Units::MB;
    } else if (splitOption.endsWith("G")) {
        result.unit = Units::GB;
    } else {
        throw std::runtime_error("Couldn't parse split option: " + splitOption.toStdString());
    }
    return result;
}

QString toString(const Split& split) {
    QString result = QString::number(split.size);
    switch (split.unit) {
    case Units::KB:
        result += "K";
        break;
    case Units::MB:
        result += "M";
        break;
    case Units::GB:
        result += "G";
        break;
    }
    return result;
}

}

OptManager::OptManager()
{
    project = false;
    log = false;
    convert = false;
    filter = false;
    convertionmode = e_ASCI;
    delimiter = QDLT_DEFAULT_EXPORT_DELIMITER;
    signature = QDLT_DEFAULT_EXPORT_SIGNATURE;
    multifilter = false;
}

OptManager::OptManager(OptManager const&)
{

}

const QStringList &OptManager::getMf4Files() const
{
    return mf4Files;
}

const QStringList &OptManager::getPcapFiles() const
{
    return pcapFiles;
}

void OptManager::printVersion(QString appname)
{
  qDebug() << "Start" << appname << "\nBuild time" << __DATE__ << __TIME__;
  qDebug() << "Version" << PACKAGE_VERSION << PACKAGE_VERSION_STATE;
}

void OptManager::printUsage()
{
    QString executable;
#if (WIN32)
    qDebug()<<"\nUsage:\n\n dlt-commander.exe [OPTIONS] [logfile] [projectfile] [filterfile] [mf4file] [pcapfile]";
    executable = " dlt-commander.exe";
#else
    qDebug()<<"\nUsage:\n\n dlt-commander [OPTIONS] [logfile] [projectfile] [filterfile] [mf4file] [pcapfile]";
    executable = " dlt-commander";
#endif

    qDebug()<<"\nOptions:\n";
    qDebug()<<" [logfile]\tLoading one or more logfiles on startup (must end with .dlt)";
    qDebug()<<" [filterfile]\tLoading filterfile on startup (must end with .dlf)";
    qDebug()<<" [pcapfile]\tImporting DLT/IPC from pcap file on startup (must end with .pcap)";
    qDebug()<<" [mf4file]\tImporting DLT/IPC from mf4 file on startup (must end with .mf4)";
    qDebug()<<" -h \t Print usage";
    qDebug()<<" -v or --version\tOnly show version and buildtime information";
    qDebug()<<" -c textfile\tConvert logfile file to textfile (logfile must end with .dlt)";
    qDebug()<<" -u\tConversion will be done in UTF8 instead of ASCII";
    qDebug()<<" -csv\tConversion will be done in CSV format";
    qDebug()<<" -d\tConversion will NOT be done, save in dlt file format again instead";
    qDebug()<<" -delimiter <character>\tThe used delimiter for CSV export (Default: "+QString(QDLT_DEFAULT_EXPORT_DELIMITER)+").";
    qDebug()<<" -signature <string>\tThe used signature for CSV export, which columns are exported (Default: "+QString(QDLT_DEFAULT_EXPORT_SIGNATURE)+").  I=Index,T=Time,S=Timestamp,O=Count,E=Ecuid,A=Apid,C=Ctid,N=SessionId,Y=Type,U=Subtype,M=Mode,R=#Args,P=Payload";
    qDebug()<<" -split <size>\t Output file size limit given in Kb, Mb or Gb (Default: infinity).";
    qDebug()<<" -multifilter\tMultifilter will generate a separate export file with the name of the filter.";
    qDebug()<<"             \t-c will define the folder name, not the filename.";
    qDebug()<<"\nExamples:\n";
    qDebug().noquote() << executable << "-c .\\trace.txt c:\\trace\\trace.dlt";
    qDebug().noquote() << executable << "-c -u .\\trace.txt c:\\trace\\trace.dlt";
    qDebug().noquote() << executable << "-d -c .\\trace.dlt c:\\trace\\trace.dlt";
    qDebug().noquote() << executable << "-csv -c .\\trace.csv c:\\trace\\trace.dlt";
    qDebug().noquote() << executable << "-csv -delimiter ; -signature TSEACP -c c:\\trace\\trace.csv c:\\trace\\trace.dlt";
    qDebug().noquote() << executable << "-d -c .\\filteredtrace.dlt c:\\filter\\filter.dlf c:\\trace\\trace.dlt";
    qDebug().noquote() << executable << "trace_1.dlt trace_2.dlt";
    qDebug().noquote() << executable << "input.pcap output.dlt";
    qDebug().noquote() << executable << "-c output.txt input.pcap";
    qDebug().noquote() << executable << "-c output.txt input1.mf4 input2.mf4";
    qDebug().noquote() << executable << "-d -split 100K c:\\trace\\trace.dlt\n -c output.dlt";
}

void OptManager::parse(QStringList *opt)
{
    QString str;

    qDebug() << "### Starting DLT Commander";

    printVersion(opt->at(0));

    qDebug() << "### Parsing Options";

    // 0==Binary 1==First Argument
    for (int i = 0; i < opt->size(); ++i)
    {
        str = opt->at(i);

        if(str.compare("-h") == 0 || str.compare("--help") == 0)
        {
            printUsage();
            exit(0);
        }
        else if(str.compare("-v") == 0 || str.compare("--version") == 0)
        {
            printVersion(opt->at(0));
            exit(0);
        }
        else if(str.compare("-c")==0)
        {
            QString c1 = opt->value(i+1);

            convertDestFile = QString("%1").arg(c1);
            // check here already if the selected file exists

            qDebug() << "Convert filename:" << convertDestFile;

            i += 1;
        }
        else if(str.compare("-delimiter")==0)
        {
            QString c1 = opt->value(i+1);

            delimiter = QString("%1").arg(c1).front().toLatin1();

            qDebug() << "Delimiter:" << delimiter;

            i += 1;
        }
        else if(str.compare("-signature")==0)
        {
            QString c1 = opt->value(i+1);

            signature = QString("%1").arg(c1).toLatin1();

            qDebug() << "Signature:" << signature;

            i += 1;
        } else if (str.compare("-split") == 0) {
            const QString c1 = opt->value(i + 1);
            try {
                split = parseSplitOption(c1);
                qDebug() << "Split size: " << toString(split.value());
            } catch (const std::runtime_error& e) {
                qDebug() << e.what();
            }

            i += 1;
        } else if (str.compare("-c") == 0) {
            qDebug() << "Convert to ASCII";

            convert = true;

            convertSourceFile = QString("%1").arg(opt->at(i+1));
            i += 1;
        } else if (str.compare("-u") == 0) {
            qDebug() << "Convert to UTF8";

            convertionmode = e_UTF8;
        } else if (str.compare("-csv") == 0) {
            qDebug() << "Convert to CSV";

            convertionmode = e_CSV;
        } else if (str.compare("-multifilter") == 0) {
            qDebug() << "Multifilter export selected.";

            multifilter = true;
        } else if (str.compare("-d") == 0) {
            qDebug() << "Convert to DLT";

            convertionmode = e_DLT;
        } else if (opt->at(i).endsWith(".dlt") || opt->at(i).endsWith(".DLT")) {
            const QString logFile = QString("%1").arg(opt->at(i));
            logFiles += logFile;
            qDebug()<< "DLT filename:" << logFile;
        } else if (opt->at(i).endsWith(".dlf") || opt->at(i).endsWith(".DLF")) {
            filterFiles += QString("%1").arg(opt->at(i));
            qDebug()<< "Filter filename:" << QString("%1").arg(opt->at(i));
        } else if (opt->at(i).endsWith(".pcap") || opt->at(i).endsWith(".PCAP")) {
            const QString pcapFile = QString("%1").arg(opt->at(i));
            pcapFiles += pcapFile;
            qDebug()<< "Pcap filename:" << pcapFile;
        } else if (opt->at(i).endsWith(".mf4") || opt->at(i).endsWith(".MF4")) {
            const QString mf4File = QString("%1").arg(opt->at(i));
            mf4Files += mf4File;
            qDebug()<< "MF4 filename:" << mf4File;
        }

    } // end of for loop
}

bool OptManager::isLogFile() const {return log;}
bool OptManager::isFilterFile() const {return filter;}
bool OptManager::isConvert()const {return convert;}
bool OptManager::isMultifilter() const {return multifilter;}
e_convertionmode OptManager::getConvertionMode() const {return convertionmode;}
QStringList OptManager::getLogFiles()const {return logFiles;}
QStringList OptManager::getFilterFiles() const {return filterFiles;}
QString OptManager::getConvertSourceFile() const {return convertSourceFile;}
QString OptManager::getConvertDestFile()const {return convertDestFile;}
char OptManager::getDelimiter() const {return delimiter;}
QString OptManager::getSignature() const {return signature;}

const std::optional<Split> &OptManager::getSplit() const
{
    return split;
}

std::size_t Split::toBytesCount() const
{
    switch (unit) {
    case Units::KB:
        return size * 1024;
    case Units::MB:
        return size * 1024 * 1024;
    case Units::GB:
        return size * 1024 * 1024 * 1024;
    default:
        return size;
    }
}
