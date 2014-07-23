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
 * \author Lassi Marttala <Lassi.LM.Marttala@partner.bmw.de
 *
 * \file dltsettingsmanager.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "dltsettingsmanager.h"
#include <QApplication>
#include <QDir>
#include <QMessageBox>

DltSettingsManager *DltSettingsManager::m_instance = NULL;

DltSettingsManager* DltSettingsManager::getInstance()
{
    if(!m_instance)
        m_instance = new DltSettingsManager();
    return m_instance;
}

void DltSettingsManager::close()
{
    if(m_instance)
        delete m_instance;
    m_instance = NULL;
}

DltSettingsManager::DltSettingsManager()
{
    /* check if directory for configuration exists */
#ifdef Q_OS_WIN
    QDir dir(QApplication::applicationDirPath()+"/config");
#else
    QDir dir("~/.dlt/config");
#endif
    if(!dir.exists())
    {
        /* directory does not exist, make it */
        if(!dir.mkpath(dir.absolutePath()))
        {
            /* creation of directory fails */
            QMessageBox::critical(0, QString("DLT Viewer"),
                                           QString("Cannot create directory to store configuration!\n\n")+dir.absolutePath(),
                                           QMessageBox::Ok,
                                           QMessageBox::Ok);
        }
    }

    settings = new QSettings(dir.absolutePath()+"/config.ini", QSettings::IniFormat);
}

DltSettingsManager::~DltSettingsManager()
{
    delete settings;
}

void DltSettingsManager::setValue(const QString &key, const QVariant &value)
{
    settings->setValue(key, value);
}

QVariant DltSettingsManager::value(const QString &key, const QVariant &defaultValue) const
{
    return settings->value(key, defaultValue);
}

void DltSettingsManager::clear()
{
    settings->clear();
}

QString DltSettingsManager::fileName() const
{
    return settings->fileName();
}
