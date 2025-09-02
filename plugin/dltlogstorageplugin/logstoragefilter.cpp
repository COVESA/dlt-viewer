/**
 * @licence app begin@
 * Copyright (C) 2015 Advanced Driver Information Technology
 *
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 *
 * \file logstoragefilter.c
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "logstoragefilter.h"

LogstorageFilter::LogstorageFilter()
{

}

LogstorageFilter::LogstorageFilter(QString apid, QString ctid, QString logLevel, QString fileName, unsigned int fileSize, unsigned int noFiles) :
    m_apid(apid),
    m_ctid(ctid),
    m_logLevel(logLevel),
    m_fileName(fileName),
    m_fileSize(fileSize),
    m_noFiles(noFiles)
{

}

QString LogstorageFilter::toText()
{
    return QString("LogAppName=" + m_apid + "\n" +
                   "ContextName=" + m_ctid + "\n" +
                   "LogLevel=" + m_logLevel + "\n" +
                   "File=" + m_fileName + "\n" +
                   "FileSize=" + QString::number(m_fileSize) + "\n" +
                   "NOFiles=" + QString::number(m_noFiles) + "\n");
}
