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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 *
 * \file qdltdefaultfilter.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtDebug>
//#include <QMessageBox>
#include <QDir>
#include <QDirIterator>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltDefaultFilter::QDltDefaultFilter()
{

}

QDltDefaultFilter::~QDltDefaultFilter()
{
    clear();
}

void QDltDefaultFilter::clear()
{
    /* delete all filter list entries */
    QDltFilterList *t;
    foreach(t,defaultFilterList)
        delete t;
    defaultFilterList.clear();

    /* delete all filter index entries */
    QDltFilterIndex *t2;
    foreach(t2,defaultFilterIndex)
        delete t2;
    defaultFilterIndex.clear();
}

void QDltDefaultFilter::load(QString path)
{
    /* delete old filter list */
    clear();

    /* load directory recursive */
    loadDirectory(path);
}

void QDltDefaultFilter::loadDirectory(QString path)
{

    QDir dir(path);

    /* set filter for default filter files */
    QStringList filters;
    filters << "*.dlf";
    dir.setNameFilters(filters);

    /* iterate through all filter in directory path */
    foreach (QString fileName, dir.entryList(QDir::Files))
    {
        /* create filter list for every filter file and load the filter file */
        QDltFilterList *filterList = new QDltFilterList();
        filterList->LoadFilter(dir.absolutePath()+"/"+fileName,true);
        defaultFilterList.append(filterList);

        /* add empty index for every filter list */
        QDltFilterIndex *filterIndex = new QDltFilterIndex();
        defaultFilterIndex.append(filterIndex);
    }
    /* Iterate over subdirectories and load files */
    QDirIterator it(path, QDir::Dirs, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (it.hasNext()) {
        QString dir = it.next();
        if (dir.endsWith("/.") || dir.endsWith("/.."))
          continue;
        loadDirectory(dir);
    }
}

void QDltDefaultFilter::clearFilterIndex()
{
    /* clear all default filter index cache */
    QDltFilterIndex *t2;
    foreach(t2,defaultFilterIndex)
        *t2 = QDltFilterIndex();
}
