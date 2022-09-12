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

#include "export_rules.h"

enum e_convertionmode
{
    e_ASCI = 0,
    e_UTF8 = 1,
    e_DLT  = 2,
    e_CSV  = 3,
    e_DDLT = 4,
};



class QDLT_EXPORT QDltOptManager
{
public:
    static QDltOptManager* getInstance();
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
    QString getLogFile();
    QString getFilterFile();
    QString getConvertSourceFile();
    QString getConvertDestFile();
    QString getPluginName();
    QString getCommandName();
    QStringList getCommandParams();

private:
    QDltOptManager();
    QDltOptManager(QDltOptManager const&);
    static QDltOptManager *instance;

    bool project;
    bool log;
    bool filter;
    bool convert;
    bool plugin;
    bool silent_mode;
    bool commandline_mode;
    e_convertionmode convertionmode;

    QString projectFile;
    QString logFile;
    QString filterFile;
    QString convertSourceFile;
    QString convertDestFile;
    QString pluginName;
    QString commandName;
    QStringList commandParams;
};

#endif //QDLTOPTMANAGER_H
