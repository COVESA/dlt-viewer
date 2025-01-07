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
 * \file optmanager.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLTOPTMANAGER_H
#define QDLTOPTMANAGER_H

#include <QStringList>
#include <QCommandLineParser>

#include "export_rules.h"

enum e_convertionmode
{
    e_ASCI = 0,
    e_UTF8 = 1,
    e_DLT  = 2,
    e_CSV  = 3,
    e_DDLT = 4,
};

enum class e_inputmode
{
    DLT    = 0,
    STREAM = 1,
    SERIAL = 2,
};

class QDLT_EXPORT QDltOptManager
{
public:
    static QDltOptManager* getInstance();
    void printUsage(const QString& helpText);
    void printVersion(QString appname);
    void parse(const QStringList& opt);
    void freeWindowsConsole();

    bool isProjectFile();
    bool isTerminate();
    bool isConvertUTF8();
    bool issilentMode();
    bool isCommandlineMode();

    e_convertionmode get_convertionmode();
    e_inputmode get_inputmode();

    QString getProjectFile();
    QStringList getLogFiles();
    QStringList getFilterFiles();
    QString getConvertDestFile();
    QString getPluginName();
    QString getCommandName();
    QStringList getCommandParams();
    QString getWorkingDirectory() const;
    const QStringList &getPrePluginCommands() const;
    const QStringList &getPostPluginCommands() const;
    const QStringList &getPcapFiles() const;
    const QStringList &getMf4Files() const;
    char getDelimiter();

    QString getHelpText() const;

    // only testing relevant
    void reset();

private:
    QDltOptManager();

private:
    bool project{false};
    bool terminate{false};
    bool silent_mode{false};
    bool commandline_mode{false};
    e_convertionmode convertionmode{e_ASCI};
    e_inputmode inputmode{e_inputmode::DLT};

    QString projectFile;
    QStringList logFiles;
    QStringList pcapFiles;
    QStringList mf4Files;
    QStringList filterFiles;
    QString convertDestFile;
    QString pluginName;
    QString commandName;
    QStringList commandParams;

    QStringList prePluginCommands; // command before loading log file
    QStringList postPluginCommands; // command after loading log file

    QString  workingDirectory;
    char delimiter{','};

    QCommandLineParser m_parser;
};

#endif //QDLTOPTMANAGER_H
