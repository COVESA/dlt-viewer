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

#include "qdltoptmanager.h"
#include "version.h"
#include <QDebug>
#include <QFileInfo>
#include <iostream>

// Global static pointer used to ensure a single instance of the class.
QDltOptManager* QDltOptManager::instance;

QDltOptManager::QDltOptManager()
{
    project = false;
    log = false;
    convert = false;
    filter = false;
    plugin = false;
    silent_mode = false;
    convertionmode = e_ASCI;
    commandline_mode = false;
}

QDltOptManager* QDltOptManager::getInstance()
{
    if (!instance)
        instance = new QDltOptManager;

    return instance;
}

QDltOptManager::QDltOptManager(QDltOptManager const&)
{

}

const QStringList &QDltOptManager::getMf4Files() const
{
    return mf4Files;
}

const QStringList &QDltOptManager::getPcapFiles() const
{
    return pcapFiles;
}

const QStringList &QDltOptManager::getPostPluginCommands() const
{
    return postPluginCommands;
}

const QStringList &QDltOptManager::getPrePluginCommands() const
{
    return prePluginCommands;
}

void QDltOptManager::printVersion(QString appname)
{
  qDebug() << "Start" << appname << "\nBuild time" << __DATE__ << __TIME__;
  qDebug() << "Version" << PACKAGE_VERSION << PACKAGE_VERSION_STATE;
  qDebug() << "**********************************************************";
}

void QDltOptManager::printUsage()
{
#if (WIN32)
    qDebug()<<"Usage: dlt-viewer.exe [OPTIONS] [logfile] [projectfile] [filterfile] [mf4file] [pcapfile]";
#else
    qDebug()<<"Usage: dlt-viewer [OPTIONS] [logfile] [projectfile] [filterfile] [mf4file] [pcapfile]";
#endif

    qDebug()<<"Options:";
    qDebug()<<" [logfile]\tLoading one or more logfiles on startup (must end with .dlt)";
    qDebug()<<" [projectfile]\tLoading project file on startup (must end with .dlp)";
    qDebug()<<" [filterfile]\tLoading filterfile on startup (must end with .dlf)";
    qDebug()<<" [pcapfile]\tImporting DLT/IPC from pcap file on startup (must end with .pcap)";
    qDebug()<<" [mf4file]\tImporting DLT/IPC from mf4 file on startup (must end with .mf4)";
    qDebug()<<" -h \t Print usage";
    qDebug()<<" -s or --silent\tEnable silent mode without warning message boxes.";
    qDebug()<<" -v or --version\tOnly show version and buildtime information";
    qDebug()<<" -c logfile textfile\tConvert logfile file to textfile (logfile must end with .dlt)";
    qDebug()<<" -u\tConversion will be done in UTF8 instead of ASCII";
    qDebug()<<" -csv\tConversion will be done in CSV format";
    qDebug()<<" -d\tConversion will NOT be done, save in dlt file format again instead";
    qDebug()<<" -dd\tConversion will NOT be done, save as decoded messages in dlt format";
    qDebug()<<" -b \"plugin|command|param1|..|param<n>\"\tExecute a plugin command with <n> parameters before loading log file.";
    qDebug()<<" -e \"plugin|command|param1|..|param<n>\"\tExecute a plugin command with <n> parameters after loading log file.";
    qDebug()<<" -w workingdirectory\tSet the working directory";
    qDebug()<<"Examples:";
    #if (WIN32)
    qDebug()<<"  dlt-viewer.exe -c c:\\trace\\trace.dlt .\\trace.txt";
    qDebug()<<"  dlt-viewer.exe -s -c -u c:\\trace\\trace.dlt .\\trace.txt";
    qDebug()<<"  dlt-viewer.exe -s -d -c c:\\trace\\trace.dlt .\\trace.dlt";
    qDebug()<<"  dlt-viewer.exe -s -p \\proj\\decodeded.dlp -dd -c c:\\trace\\trace.dlt .\\trace.dlt";
    qDebug()<<"  dlt-viewer.exe -s -csv -c c:\\trace\\trace.dlt .\\trace.csv";
    qDebug()<<"  dlt-viewer.exe -s -d -f c:\\filter\\filter.dlf -c c:\\trace\\trace.dlt .\\filteredtrace.dlt";
    qDebug()<<"  dlt-viewer.exe -p c:\\proj\\export.dlp -l c:\\trace\\trace.dlt -e \"Filetransfer Plugin|export|ftransferdir\"";
    qDebug()<<"  dlt-viewer.exe trace_1.dlt trace_2.dlt";
    #else
    qDebug()<<"  dlt-viewer -c ./traces/trace.dlt ./trace.txt";
    qDebug()<<"  dlt-viewer -s -c -u ./trace/trace.dlt ./trace.txt";
    qDebug()<<"  dlt-viewer -s -d -c ./trace/trace.dlt ./trace.dlt";
    qDebug()<<"  dlt-viewer -s -p ./proj/decodeded.dlp -dd -c ./trace/trace.dlt ./trace.dlt";
    qDebug()<<"  dlt-viewer -s -csv -c ./trace/trace.dlt ./trace.csv";
    qDebug()<<"  dlt-viewer -s -d -f ./filter/filter.dlf -c ./trace/trace.dlt ./filteredtrace.dlt";
    qDebug()<<"  dlt-viewer -p ./proj/export.dlp -l ./trace/trace.dlt -e \"Filetransfer Plugin|export|./ftransferdir\"";
    qDebug()<<"  dlt-viewer trace_1.dlt trace_2.dlt";
    #endif
}

void QDltOptManager::parse(QStringList *opt)
{
    QString str;

     /* On Windows we do not want to open a console in case
      * we start the application e.g. from file explorer.
      * Unfortunateley Windows opens a console anyway.
      * So we have to close it in this case
     */
     #if (WIN32)
     if ( opt->size() < 2)
     {
      FreeConsole();
     }
     #endif

     /* the default parameter - exactly one parameter - should either be
      * a dlt or a dlp file, so this enables the "doubleclick" feature
      */
     //str = opt->at(0); && ( str.compare("-h)") != 0 || str.compare("-v") !=0  )
     if(opt->size()==2  )
       {
           if(opt->at(1).endsWith(".dlp") || opt->at(1).endsWith(".DLP"))
           {
               projectFile = QString("%1").arg(opt->at(1));
               project = true;
               qDebug()<< "Loading projectfile " << projectFile;
               return;
           }
           if(opt->at(1).endsWith(".dlt") || opt->at(1).endsWith(".DLT"))
           {
               const QString logFile = QString("%1").arg(opt->at(1));
               logFiles += logFile;
               log = true;
               qDebug()<< "Loading logfile " << logFile;
               return;
           }
       }

    // 0==Binary 1==First Argument
    for (int i = 0; i < opt->size(); ++i)
     {
        str = opt->at(i);

        if(str.compare("-h") == 0 || str.compare("--help") == 0)
          {
            printUsage();
            exit(0);
          }
        else if(str.compare("-s") == 0 || str.compare("--silent") == 0)
         {
            if ( silent_mode == false)
            {
            silent_mode = true;
            qDebug() << "Enable silent mode";
            }
         }
        else if(str.compare("-v") == 0 || str.compare("--version") == 0)
         {
            printVersion(opt->at(0));
            exit(0);
         }
        else if(str.compare("-c")==0)
         {
            if (log == true)
            {
              qDebug() << "\nError: Can't use -l and -c at once\n";
              printUsage();
              exit(-1);
            }
            QString c1 = opt->value(i+1);
            QString c2 = opt->value(i+2);

            if(c1!=nullptr && (c1.endsWith(".dlt")||c1.endsWith(".DLT")) && c2!=nullptr)
             {
                convertSourceFile = QString("%1").arg(c1);
                convertDestFile = QString("%1").arg(c2);
                // check here already if the selected file exists

                if(QFileInfo::exists(convertSourceFile))
                 {
                    qDebug() << "Converting " << convertSourceFile << "to" << convertDestFile;
                    convert = true;
                    commandline_mode = true;
                 }
                else
                 {
                    qDebug() << "Convertfile source"  << convertSourceFile << "does not exist";
                    exit(-1);
                 }
             }
            else
             {
                qDebug()<<"Error occured during processing of command line option \"-c\"";
                printUsage();
                exit(-1);
             }
            i += 2;
         }
        else if(str.compare("-u")==0)
         {
            convertionmode = e_UTF8;
         }
        else if(str.compare("-csv")==0)
         {
            convertionmode = e_CSV;
         }
        else if(str.compare("-d")==0)
         {
            convertionmode = e_DLT;
         }
        else if(str.compare("-dd")==0)
         {
            convertionmode = e_DDLT;
         }
        else if(str.compare("-e")==0)
         {
            QString c = opt->value(i+1);
            postPluginCommands += c;
            QStringList args = c.split("|");
            commandline_mode = true;
            if(c != nullptr && args.size() > 1)
             {
                pluginName = args.at(0);
                commandName = args.at(1);
                args.removeAt(0);
                args.removeAt(0);
                commandParams = args;
                plugin = true;
             }
            else
             {
                qDebug()<<"Error occured during processing of command line option \"-e\"";
                printUsage();
                exit(-1);
             }
            ++i;
         }
        else if(str.compare("-b")==0)
         {
            QString c = opt->value(i+1);
            prePluginCommands += c;
            commandline_mode = true;
            ++i;
         }
        else if (str.compare("-w") == 0)
        {
            workingDirectory = opt->value(i+1);
            ++i;
        }
        else if(opt->at(i).endsWith(".dlt") || opt->at(i).endsWith(".DLT"))
        {
            if (convert == true)
            {
                qDebug() << "\nError: Can't use -c and multiple logfiles once\n";
                printUsage();
                exit(-1);
            }

            const QString logFile = QString("%1").arg(opt->at(i));
            logFiles += logFile;
            log = true;
            qDebug()<< "Loading logfile " << logFile;
        }
        else if(opt->at(i).endsWith(".dlp") || opt->at(i).endsWith(".DLP"))
        {
            if (project == true)
            {
                qDebug() << "\nError: Can only load one project file\n";
                printUsage();
                exit(-1);
            }

            projectFile = QString("%1").arg(opt->at(i));
            project = true;
            qDebug()<< "Loading projectfile " << projectFile;
        }
        else if(opt->at(i).endsWith(".dlf") || opt->at(i).endsWith(".DLF"))
        {
            if (filter == true)
            {
                qDebug() << "\nError: Can't load multiple filter files once\n";
                printUsage();
                exit(-1);
            }

            filterFile = QString("%1").arg(opt->at(i));
            filter = true;
            qDebug()<< "Loading filterfile " << filterFile;
        }
        else if(opt->at(i).endsWith(".pcap") || opt->at(i).endsWith(".PCAP"))
        {
            const QString pcapFile = QString("%1").arg(opt->at(i));
            pcapFiles += pcapFile;
            qDebug()<< "Importing pcapfile" << pcapFile;
        }
        else if(opt->at(i).endsWith(".mf4") || opt->at(i).endsWith(".MF4"))
        {
            const QString mf4File = QString("%1").arg(opt->at(i));
            mf4Files += mf4File;
            qDebug()<< "Importing mf4file" << mf4File;
        }

     } // end of for loop
    printVersion(opt->at(0));
}

bool QDltOptManager::isProjectFile(){ return project;}
bool QDltOptManager::isLogFile(){return log;}
bool QDltOptManager::isFilterFile(){return filter;}
bool QDltOptManager::isConvert(){return convert;}
bool QDltOptManager::isPlugin(){return plugin;}
bool QDltOptManager::issilentMode(){return silent_mode;}
bool QDltOptManager::isCommandlineMode(){return commandline_mode;}

e_convertionmode QDltOptManager::get_convertionmode()
{
    return convertionmode;
}

QString QDltOptManager::getProjectFile(){return projectFile;}
QStringList QDltOptManager::getLogFiles(){return logFiles;}
QString QDltOptManager::getFilterFile(){return filterFile;}
QString QDltOptManager::getConvertSourceFile(){return convertSourceFile;}
QString QDltOptManager::getConvertDestFile(){return convertDestFile;}
QString QDltOptManager::getPluginName(){return pluginName;}
QString QDltOptManager::getCommandName(){return commandName;}
QStringList QDltOptManager::getCommandParams(){return commandParams;}
QString QDltOptManager::getWorkingDirectory() const { return workingDirectory; }
