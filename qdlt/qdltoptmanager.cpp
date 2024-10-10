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

// Global static pointer used to ensure a single instance of the class.
QDltOptManager* QDltOptManager::instance;

QDltOptManager::QDltOptManager()
{
    project = false;
    silent_mode = false;
    terminate=false;
    convertionmode = e_ASCI;
    commandline_mode = false;
    delimiter=',';
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
    qDebug() << "Executable Name:" << appname;
    qDebug() << "Build time:" << __DATE__ << __TIME__;
    qDebug() << "Version:" << PACKAGE_VERSION << PACKAGE_VERSION_STATE;
}

void QDltOptManager::printUsage()
{
#if (WIN32)
    qDebug()<<"Usage: dlt-viewer.exe [OPTIONS] [logfile] [projectfile] [filterfile] [mf4file] [pcapfile]";
#else
    qDebug()<<"Usage: dlt-viewer [OPTIONS] [logfile] [projectfile] [filterfile] [mf4file] [pcapfile]";
#endif

    qDebug()<<"\nOptions:";
    qDebug()<<" [logfile]\tLoading one or more logfiles on startup (must end with .dlt)";
    qDebug()<<" [projectfile]\tLoading project file on startup (must end with .dlp)";
    qDebug()<<" [filterfile]\tLoading filterfile on startup (must end with .dlf)";
    qDebug()<<" [pcapfile]\tImporting DLT/IPC from pcap file on startup (must end with .pcap)";
    qDebug()<<" [mf4file]\tImporting DLT/IPC from mf4 file on startup (must end with .mf4)";
    qDebug()<<" -h or --help\tPrint usage";
    qDebug()<<" -c textfile\tConvert logfile file to textfile";
    qDebug()<<" -u\tConversion will be done in UTF8 instead of ASCII";
    qDebug()<<" -csv\tConversion will be done in CSV format";
    qDebug()<<" -d\tConversion will NOT be done, save in dlt file format again instead";
    qDebug()<<" -dd\tConversion will NOT be done, save as decoded messages in dlt format";
    qDebug()<<" -b \"plugin|command|param1|..|param<n>\"\tExecute a plugin command with <n> parameters before loading log file.";
    qDebug()<<" -e \"plugin|command|param1|..|param<n>\"\tExecute a plugin command with <n> parameters after loading log file.";
    qDebug()<<" -s or --silent\tEnable silent mode without any GUI. Ideal for commandline usage.";
    qDebug()<<" -stream\tTreat the input logfiles as DLT stream instead of DLT files.";
    qDebug()<<" -t or --terminate\tTerminate DLT Viewer after command line execution.";
    qDebug()<<" -v or --version\tOnly show version and buildtime information";
    qDebug()<<" -w workingdirectory\tSet the working directory";
    qDebug()<<" -delimiter <character>\tThe used delimiter for CSV export (Default: ,).";
    qDebug()<<"\nExamples:";
    qDebug()<<"  dlt-viewer.exe -t -c output.txt input.dlt";
    qDebug()<<"  dlt-viewer.exe -t -s -u -c output.txt input.dlt";
    qDebug()<<"  dlt-viewer.exe -t -s -d -c output.dlt input.dlt";
    qDebug()<<"  dlt-viewer.exe -t -s decoded.dlp -dd -c output.dlt input.dlt ";
    qDebug()<<"  dlt-viewer.exe -t -s -csv -c output.csv input.dlt";
    qDebug()<<"  dlt-viewer.exe -t -s -d filter.dlf -c output.dlt input.dlt";
    qDebug()<<"  dlt-viewer.exe -p export.dlp -e \"Filetransfer Plugin|export|ftransferdir\" input.dlt";
    qDebug()<<"  dlt-viewer.exe input1.dlt input2.dlt";
    qDebug()<<"  dlt-viewer.exe -t -c output.txt input.pcap";
    qDebug()<<"  dlt-viewer.exe -t -c output.txt input1.mf4 input2.mf4";
}

void QDltOptManager::parse(QStringList *opt)
{
    QString str;

    qDebug() << "### Starting DLT Viewer";

    printVersion(opt->at(0));

    qDebug() << "### Parsing Options";

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
               qDebug()<< "Project filename:" << projectFile;
               return;
           }
           if(opt->at(1).endsWith(".dlt") || opt->at(1).endsWith(".DLT"))
           {
               const QString logFile = QString("%1").arg(opt->at(1));
               logFiles += logFile;
               qDebug()<< "DLT filename:" << logFile;
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
            qDebug() << "Silent mode enabled";
            }
         }
        else if(str.compare("-v") == 0 || str.compare("--version") == 0)
         {
            printVersion(opt->at(0));
            exit(0);
         }
         else if(str.compare("-t") == 0 || str.compare("--terminate") == 0)
         {
             terminate = true;
             commandline_mode = true;
         }
        else if(str.compare("-c")==0)
         {
            QString c1 = opt->value(i+1);

            convertDestFile = QString("%1").arg(c1);
            // check here already if the selected file exists

            qDebug() << "Convert filename:" << convertDestFile;
            commandline_mode = true;

            i += 1;
         }
         else if(str.compare("-delimiter")==0)
         {
             QString c1 = opt->value(i+1);

             delimiter = QString("%1").arg(c1).front().toLatin1();

             qDebug() << "Delimiter:" << delimiter;

             i += 1;
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
        else if(str.compare("-stream")==0)
         {
            inputmode = e_inputmode::STREAM;
         }
        else if(str.compare("-e")==0)
         {
            QString c = opt->value(i+1);
            postPluginCommands += c;
            commandline_mode = true;
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
            const QString logFile = QString("%1").arg(opt->at(i));
            logFiles += logFile;
            qDebug()<< "DLT filename:" << logFile;
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
            qDebug()<< "Project filename:" << projectFile;
        }
        else if(opt->at(i).endsWith(".dlf") || opt->at(i).endsWith(".DLF"))
        {
            filterFiles += QString("%1").arg(opt->at(i));
            qDebug()<< "Filter filename:" << QString("%1").arg(opt->at(i));
        }
        else if(opt->at(i).endsWith(".pcap") || opt->at(i).endsWith(".PCAP"))
        {
            const QString pcapFile = QString("%1").arg(opt->at(i));
            pcapFiles += pcapFile;
            qDebug()<< "Pcap filename:" << pcapFile;
        }
        else if(opt->at(i).endsWith(".mf4") || opt->at(i).endsWith(".MF4"))
        {
            const QString mf4File = QString("%1").arg(opt->at(i));
            mf4Files += mf4File;
            qDebug()<< "MF4 filename:" << mf4File;
        }

     } // end of for loop

    /* On Windows we do not want to open a console in case
      * we start the application e.g. from file explorer.
      * Unfortunateley Windows opens a console anyway.
      * So we have to close it in this case
     */
    #if (WIN32)
        if (!commandline_mode)
        {
            HWND consoleWnd = GetConsoleWindow();
            DWORD dwProcessId;
            GetWindowThreadProcessId(consoleWnd, &dwProcessId);
            if (GetCurrentProcessId() == dwProcessId)
            {
                // user launched the application with a double click from explorer: we do not need console
                FreeConsole();
            }
        }
    #endif
}

bool QDltOptManager::isProjectFile(){ return project;}
bool QDltOptManager::isTerminate(){return terminate;}
bool QDltOptManager::issilentMode(){return silent_mode;}
bool QDltOptManager::isCommandlineMode(){return commandline_mode;}
e_convertionmode QDltOptManager::get_convertionmode(){return convertionmode;}
e_inputmode QDltOptManager::get_inputmode(){return inputmode;}
QString QDltOptManager::getProjectFile(){return projectFile;}
QStringList QDltOptManager::getLogFiles(){return logFiles;}
QStringList QDltOptManager::getFilterFiles(){return filterFiles;}
QString QDltOptManager::getConvertDestFile(){return convertDestFile;}
QString QDltOptManager::getPluginName(){return pluginName;}
QString QDltOptManager::getCommandName(){return commandName;}
QStringList QDltOptManager::getCommandParams(){return commandParams;}
QString QDltOptManager::getWorkingDirectory() const { return workingDirectory; }
char QDltOptManager::getDelimiter(){return delimiter;}
