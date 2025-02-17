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

#include <optional>

enum e_convertionmode
{
    e_ASCI = 0,
    e_UTF8 = 1,
    e_DLT  = 2,
    e_CSV  = 3,
};

enum class Units {
    KB, // kilobytes
    MB, // megabytes
    GB  // gigabytes
};

struct Split {
    std::size_t size;
    Units unit;

    std::size_t toBytesCount() const;
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

    bool isLogFile() const;
    bool isFilterFile() const;
    bool isConvert() const;
    bool isConvertUTF8() const;
    bool isMultifilter() const;

    e_convertionmode getConvertionMode() const;
    QStringList getLogFiles()const ;
    QStringList getFilterFiles() const;
    QString getConvertSourceFile() const;
    QString getConvertDestFile() const;
    char getDelimiter() const;
    const std::optional<Split>& getSplit() const;
    QString getSignature() const;

    const QStringList &getPcapFiles() const;
    const QStringList &getMf4Files() const;

private:
    bool project;
    bool log;
    bool filter;
    bool convert;
    bool multifilter;
    //split size
    std::optional<Split> split;

    e_convertionmode convertionmode;

    QStringList logFiles;
    QStringList pcapFiles;
    QStringList mf4Files;
    QStringList filterFiles;
    QString convertSourceFile;
    QString convertDestFile;
    char delimiter;
    QString signature;
};

#endif // OPTMANAGER_H
