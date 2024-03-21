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

#ifndef OPTMANAGER_H
#define OPTMANAGER_H

#include <QStringList>

enum e_convertionmode
{
    e_ASCI = 0,
    e_UTF8 = 1,
    e_DLT  = 2,
    e_CSV  = 3,
    e_DDLT = 4,
};



class OptManager
{
public:
    OptManager();
    OptManager(OptManager const&);

    //static OptManager* getInstance();
    void printUsage();
    void printVersion(QString appname);
    void parse(QStringList *opt);

    bool isProjectFile();
    bool isLogFile();
    bool isFilterFile();
    bool isConvert();
    bool isConvertUTF8();
    bool isPlugin();
    bool issilentMode();
    bool isCommandlineMode();

    e_convertionmode get_convertionmode();

    QString getProjectFile();
    QStringList getLogFiles();
    QString getFilterFile();
    QString getConvertSourceFile();
    QString getConvertDestFile();
    QString getPluginName();
    QString getCommandName();
    QStringList getCommandParams();
    QString getWorkingDirectory() const;
    const QStringList &getPrePluginCommands() const;
    const QStringList &getPostPluginCommands() const;

    const QStringList &getPcapFiles() const;

    const QStringList &getMf4Files() const;

private:
    //static OptManager *instance;

    bool project;
    bool log;
    bool filter;
    bool convert;
    bool plugin;
    bool silent_mode;
    bool commandline_mode;
    e_convertionmode convertionmode;

    QString projectFile;
    QStringList logFiles;
    QStringList pcapFiles;
    QStringList mf4Files;
    QString filterFile;
    QString convertSourceFile;
    QString convertDestFile;
    QString pluginName;
    QString commandName;
    QStringList commandParams;

    QStringList prePluginCommands; // command before loading log file
    QStringList postPluginCommands; // command after loading log file

    QString  workingDirectory;
};

#endif // OPTMANAGER_H
