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

QDltOptManager::QDltOptManager()
{
    m_parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    m_parser.addPositionalArgument("logfile", "Loading one or more logfiles on startup (must end with .dlt)");
    m_parser.addPositionalArgument("projectfile", "Loading project file on startup (must end with .dlp)");
    m_parser.addPositionalArgument("filterfile", "Loading filterfile on startup (must end with .dlf)");
    m_parser.addPositionalArgument("pcapfile", "Importing DLT/IPC from pcap file on startup (must end with .pcap)");
    m_parser.addPositionalArgument("mf4file", "Importing DLT/IPC from mf4 file on startup (must end with .mf4)");
    m_parser.addOptions({
        {"c", "Convert logfile file to <textfile>", "textfile"},
        {"u", "Conversion will be done in UTF8 instead of ASCII"},
        {"csv", "Conversion will be done in CSV format"},
        {"d", "Conversion will NOT be done, save in dlt file format again instead"},
        {"dd", "Conversion will NOT be done, save as decoded messages in dlt format"},
        {"b", "Execute a plugin command with <n> parameters before loading log file.", "plugin|command|param1|..|param<n>"},
        {"e", "Execute a plugin command with <n> parameters after loading log file.", "plugin|command|param1|..|param<n>"},
        {QStringList() << "s" << "silent", "Enable silent mode without any GUI. Ideal for commandline usage."},
        {"stream", "Treat the input logfiles as DLT stream instead of DLT files."},
        {QStringList() << "t" << "terminate", "Terminate DLT Viewer after command line execution."},
        {"w", "Set the working directory", "workingdirectory"},
        {"delimiter", "The used delimiter for CSV export (Default: ,).", "character"},
        {QStringList() << "h" << "help", "Print this help message."},
        {QStringList() << "v" << "version", "Print the version."}
    });

}

QDltOptManager* QDltOptManager::getInstance()
{
    static QDltOptManager instance;

    return &instance;
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

void QDltOptManager::printUsage(const QString& helpText)
{
    qDebug().noquote() << helpText;
    qDebug() << "\nExamples:";
    qDebug() << "  dlt-viewer.exe -t -c output.txt input.dlt";
    qDebug() << "  dlt-viewer.exe -t -s -u -c output.txt input.dlt";
    qDebug() << "  dlt-viewer.exe -t -s -d -c output.dlt input.dlt";
    qDebug() << "  dlt-viewer.exe -t -s decoded.dlp -dd -c output.dlt input.dlt ";
    qDebug() << "  dlt-viewer.exe -t -s -csv -c output.csv input.dlt";
    qDebug() << "  dlt-viewer.exe -t -s -d filter.dlf -c output.dlt input.dlt";
    qDebug() << "  dlt-viewer.exe -p export.dlp -e \"Filetransfer Plugin|export|ftransferdir\" input.dlt";
    qDebug() << "  dlt-viewer.exe input1.dlt input2.dlt";
    qDebug() << "  dlt-viewer.exe -t -c output.txt input.pcap";
    qDebug() << "  dlt-viewer.exe -t -c output.txt input1.mf4 input2.mf4";
}

void QDltOptManager::freeWindowsConsole()
{
#if (WIN32 || WIN64)
    HWND consoleWnd = GetConsoleWindow();
    DWORD dwProcessId;
    GetWindowThreadProcessId(consoleWnd, &dwProcessId);
    if (GetCurrentProcessId() == dwProcessId)
    {
        FreeConsole();
    }
#endif
}

void QDltOptManager::parse(const QStringList& args)
{
    m_parser.parse(args);

    qDebug() << "### Starting DLT Viewer";

    printVersion(args.at(0));

     /* the default parameter - exactly one parameter - should either be
      * a dlt or a dlp file, so this enables the "doubleclick" feature
      */

    if (m_parser.optionNames().isEmpty() && m_parser.positionalArguments().size() == 1)
    {
        const QString& arg = m_parser.positionalArguments().at(0);
        bool closeConsole = false;
        if(arg.endsWith(".dlp") || arg.endsWith(".DLP"))
        {
            projectFile = arg;
            project = true;
            qDebug()<< "Project filename:" << projectFile;
            closeConsole = true;
        }
        if (arg.endsWith(".dlt") || arg.endsWith(".DLT"))
        {
            const QString logFile = arg;
            logFiles += logFile;
            qDebug()<< "DLT filename:" << logFile;
            closeConsole = true;
        }

        if(closeConsole)
        {
            // user launched the application with a double click on a dlt/project file: we do not need console
            freeWindowsConsole();
            return;
        }
    }

    if (m_parser.isSet("help")) {
        printUsage(m_parser.helpText());
        exit(0);
    }

    if (m_parser.isSet("silent")) {
        silent_mode = true;
        qDebug() << "Silent mode enabled";
    }

    if (m_parser.isSet("version")) {
        // version is already printed above
        exit(0);
    }

    if (m_parser.isSet("terminate")) {
        terminate = true;
        commandline_mode = true;
    }

    if (m_parser.isSet("c")) {
        convertDestFile = m_parser.value("c");
        qDebug() << "Convert filename:" << convertDestFile;
        commandline_mode = true;
    }

    if (m_parser.isSet("delimiter")) {
        delimiter = m_parser.value("delimiter").front().toLatin1();
        qDebug() << "Delimiter:" << delimiter;
    }

    if (m_parser.isSet("u")) {
        convertionmode = e_UTF8;
    }

    if (m_parser.isSet("csv")) {
        convertionmode = e_CSV;
    }

    if (m_parser.isSet("d")) {
        convertionmode = e_DLT;
    }

    if (m_parser.isSet("dd")) {
        convertionmode = e_DDLT;
    }

    if (m_parser.isSet("stream")) {
        inputmode = e_inputmode::STREAM;
    }

    if (m_parser.isSet("e")) {
        postPluginCommands += m_parser.values("e");
        commandline_mode = true;
    }

    if (m_parser.isSet("b")) {
        prePluginCommands += m_parser.values("b");
        commandline_mode = true;
    }

    if (m_parser.isSet("w")) {
        workingDirectory = m_parser.value("w");
    }

    QStringList positionalArguments = m_parser.positionalArguments();
    for (const QString &arg : positionalArguments)
    {
        if(arg.endsWith(".dlt") || arg.endsWith(".DLT"))
        {
            const QString logFile = arg;
            logFiles += logFile;
            qDebug()<< "DLT filename:" << logFile;
        }
        else if(arg.endsWith(".dlp") || arg.endsWith(".DLP"))        {
            if (project == true)
            {
                qDebug() << "\nError: Can only load one project file\n";
                printUsage(m_parser.helpText());
                exit(-1);
            }

            projectFile = arg;
            project = true;
            qDebug()<< "Project filename:" << projectFile;
        }
        else if(arg.endsWith(".dlf") || arg.endsWith(".DLF"))
        {
            filterFiles += arg;
            qDebug()<< "Filter filename:" << arg;
        }
        else if(arg.endsWith(".pcap") || arg.endsWith(".PCAP"))
        {
            const QString pcapFile = arg;
            pcapFiles += pcapFile;
            qDebug()<< "Pcap filename:" << pcapFile;
        }
        else if(arg.endsWith(".mf4") || arg.endsWith(".MF4"))
        {
            const QString mf4File = arg;
            mf4Files += mf4File;
            qDebug()<< "MF4 filename:" << mf4File;
        }
    }

    /* On Windows we do not want to open a console in case
      * we start the application e.g. from file explorer.
      * Unfortunateley Windows opens a console anyway.
      * So we have to close it in this case
     */
    if (!commandline_mode)
    {
        // user launched the application with a double click from explorer: we do not need console
        freeWindowsConsole();
    }
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

QString QDltOptManager::getHelpText() const
{
    return m_parser.helpText();
}

void QDltOptManager::reset()
{
    project = false;
    terminate = false;
    silent_mode = false;
    commandline_mode = false;
    convertionmode = e_ASCI;
    inputmode = e_inputmode::DLT;
    projectFile.clear();
    logFiles.clear();
    filterFiles.clear();
    convertDestFile.clear();
    pluginName.clear();
    commandName.clear();
    commandParams.clear();
    prePluginCommands.clear();
    postPluginCommands.clear();
    workingDirectory.clear();
    delimiter=',';
    pcapFiles.clear();
    mf4Files.clear();
}
