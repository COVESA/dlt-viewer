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
 * \file qdltfilterindex.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef QDLT_FILTER_INDEX_H
#define QDLT_FILTER_INDEX_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"

class QDLT_EXPORT QDltFilterIndex
{
public:

    QString dltFileName;

    QString filterFileName;

    int allIndexSize;

    //! Index of all DLT messages matching filter.
    /*!
      Index contains positions of DLT messages in indexAll.
    */
    QList<unsigned long> indexFilter;

protected:

private:

};

#endif // QDLT_FILTER_INDEX_H
