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
 * \file qdltdefaultfilter.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef QDLT_DEFAULT_FILTER_H
#define QDLT_DEFAULT_FILTER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "export_rules.h"


class QDLT_EXPORT QDltDefaultFilter
{
public:

    //! The constructor.
    /*!
    */
    QDltDefaultFilter();

    //! The destructor.
    /*!
    */
    ~QDltDefaultFilter();

    //! Delete all filters and markers.
    /*!
      This includes all positive and negative filters and markers.
    */
    void clear();

    //! Load the filter list.
    /*!
      The old filter list is deleted.
    */
    void load(QString path);

    //! Append load to the filter list.
    /*!
      The old filter list is kept.
    */
    void loadDirectory(QString path);

    //! Clear the default filter index cache.
    /*!
      The filter index are cleared.
    */
    void clearFilterIndex();

    /* Default Filter List */
    QList<QDltFilterList*> defaultFilterList;

    /* Default Filter Index */
    QList<QDltFilterIndex*> defaultFilterIndex;

protected:
private:


};

#endif // QDLT_DEFAULT_FILTER_H
