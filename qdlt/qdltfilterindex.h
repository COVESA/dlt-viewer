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
 * \file qdltfilterindex.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLT_FILTER_INDEX_H
#define QDLT_FILTER_INDEX_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"
#include "qdltfilterlist.h"

class QDLT_EXPORT QDltFilterIndex
{
public:

    QString dltFileName;
    int allIndexSize;

    QDltFilterList filterList;

    //! Index of all DLT messages matching filter.
    /*!
      Index contains positions of DLT messages in indexAll.
    */
    QVector<qint64> indexFilter;

    //! Set the filter index cache.
    /*!
    */
    void setIndexFilter(QVector<qint64> _indexFilter);

    //! Set filename for plausibility check.
    /*!
    */
    void setDltFileName(QString _dltFileName);

    //! Set the dlt file messages size for plausibility check.
    /*!
    */
    void setAllIndexSize(int _allIndexSize);

protected:

private:

};

#endif // QDLT_FILTER_INDEX_H
