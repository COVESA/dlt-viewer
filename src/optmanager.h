/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file optmanager.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef OPTMANAGER_H
#define OPTMANAGER_H

#include <QStringList>

class OptManager
{
public:
    static OptManager* getInstance();
    void printUsage();
    void parse(QStringList *opt);

    bool isProjectFile();
    bool isLogFile();
    bool isFilterFile();
    bool isConvert();
    bool isConvertUTF8();
    bool isPlugin();
    bool issilentMode();

    QString getProjectFile();
    QString getLogFile();
    QString getFilterFile();
    QString getConvertSourceFile();
    QString getConvertDestFile();
    QString getPluginName();
    QString getCommandName();
    QStringList getCommandParams();

private:
    OptManager();
    OptManager(OptManager const&);
    static OptManager *instance;

    bool project;
    bool log;
    bool filter;
    bool convert;
    bool convertUTF8;
    bool plugin;
    bool silent_mode;
    QString projectFile;
    QString logFile;
    QString filterFile;
    QString convertSourceFile;
    QString convertDestFile;
    QString pluginName;
    QString commandName;
    QStringList commandParams;
};

#endif // OPTMANAGER_H
