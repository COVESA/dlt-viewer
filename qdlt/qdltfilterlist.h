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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 *
 * \file qdlt.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLT_FILTER_LIST_H
#define QDLT_FILTER_LIST_H

#include "export_rules.h"
#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
#ifdef USECOLOR
#include <QColor>
#endif
#include <QMutex>
#include <time.h>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class QDLT_EXPORT QDltFilterList
{
public:

    //! List of filters.
    QList<QDltFilter*> filters;

    //! The constructor.
    /*!
    */
    QDltFilterList();

    QDltFilterList(const QDltFilterList &other);

    //! The destructor.
    /*!
    */
    ~QDltFilterList();

    //! Copy operator.
    QDltFilterList& operator= (QDltFilterList const& _filterList);

    //! Delete all filters and markers.
    /*!
      This includes all positive and negative filters and markers.
    */
    void clearFilter();

    //! Add a filter to the filter list.
    /*!
      \param filter the filter configuration
    */
    void addFilter(QDltFilter *_filter);

    //! Check if message will be marked.
    /*!
      Colours used are:
      1 = green
      2 = red
      3 = yellow
      4 = blue
      5 = light grey
      6 = dark grey
      \param msg The messages to be marked
      \return 0 if message will not be marked, colour if message will be marked
    */
#ifdef USECOLOR
    QColor checkMarker(QDltMsg &msg);
#else
    QString checkMarker(QDltMsg &msg);
#endif



    //! Check if message matches the filter.
    /*!
      \param msg The message to be checked
      \return true if message will be displayed, false if message will be filtered out
    */
    bool checkFilter(QDltMsg &msg);

    //! Save the filter.
    /*!
    */
    bool SaveFilter(QString _filename);

    //! Load the filter list.
    /*!
    */
    bool LoadFilter(QString _filename,bool replace);

    //! Save the filter in a byte array and create a MD5 checksum over the byte array.
    /*!
    */
    QByteArray createMD5();

    //! Return the filename.
    /*!
    */
    QString getFilename() const { return filename; }

    //! Update the presorted list for performance improvement.
    /*!
    */
    void updateSortedFilter();

protected:
private:

    //! The filename of the filter list including complete path.
    QString filename;

    //! List of mfilters.
    QList<QDltFilter*> mfilters;

    //! List of pfilters.
    QList<QDltFilter*> pfilters;

    //! List of nfilters.
    QList<QDltFilter*> nfilters;

};

#endif // QDLT_FILTER_LIST_H
