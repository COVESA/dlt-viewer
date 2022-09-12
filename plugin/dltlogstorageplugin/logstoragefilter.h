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
 * \file logstoragefilter.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */


#ifndef LOGSTORAGEFILTER_H
#define LOGSTORAGEFILTER_H

#include <QString>

/**
 * LogstorageFilter class
 *
 * Describes a certain filter configuration.
 */
class LogstorageFilter
{
public:
    LogstorageFilter();
    LogstorageFilter(QString apid, QString ctid, QString logLevel, QString fileName, unsigned int fileSize, unsigned int noFiles);

    QString getApid() const { return m_apid; }
    void setApid(const QString &value) { m_apid = value; }

    QString getCtid() const { return m_ctid; }
    void setCtid(const QString &value) { m_ctid = value; }

    QString getLogLevel() const { return m_logLevel; }
    void setLogLevel(const QString &value) { m_logLevel = value; }

    QString getFileName() const { return m_fileName; }
    void setFileName(const QString &value) { m_fileName = value; }

    unsigned int getFileSize() const { return m_fileSize; }
    void setFileSize(unsigned int value) { m_fileSize = value; }

    unsigned int getNoFiles() const { return m_noFiles; }
    void setNoFiles(unsigned int value) { m_noFiles = value; }


    QString toText();


private:
    QString m_apid;
    QString m_ctid;
    QString m_logLevel;
    QString m_fileName;
    unsigned int m_fileSize;
    unsigned int m_noFiles;
};

#endif // LOGSTORAGEFILTER_H
