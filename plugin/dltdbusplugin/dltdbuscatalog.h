/**
 * @licence app begin@
 * Copyright (C) 2011-2014  BMW AG
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
 * \file dltdbuscatalog.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef DLTDBUSCATALOG_H
#define DLTDBUSCATALOG_H

#include <QList>
#include <QString>
#include <QStringList>

class DltDBusCatalogArgument
{
public:
    DltDBusCatalogArgument();
    ~DltDBusCatalogArgument();

    QString name;
    QString type;
    QString access;
    QString direction;
};

class DltDBusCatalogFunction
{
public:
    DltDBusCatalogFunction();
    ~DltDBusCatalogFunction();

    QList<DltDBusCatalogArgument*> arguments;

    QString name;
};

class DltDBusCatalogInterface
{
public:
    DltDBusCatalogInterface();
    ~DltDBusCatalogInterface();

    QList<DltDBusCatalogFunction*> functions;

    QString name;
};


class DltDBusCatalog
{
public:
    DltDBusCatalog();
    ~DltDBusCatalog();

    void clear();

    bool parse(QString filename);
    QStringList info();

    QList<DltDBusCatalogInterface*> interfaces;
    QString errorString;
};

#endif // DLTDBUSCATALOG_H
