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
 * \file qdltfilterindex.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtDebug>

#include "qdltfilterindex.h"

extern "C"
{
#include "dlt_common.h"
}

void QDltFilterIndex::setIndexFilter(QVector<qint64> _indexFilter)
{
    indexFilter = _indexFilter;
}

void QDltFilterIndex::setDltFileName(QString _dltFileName)
{
    dltFileName = _dltFileName;
}

void QDltFilterIndex::setAllIndexSize(int _allIndexSize)
{
    allIndexSize = _allIndexSize;
}
