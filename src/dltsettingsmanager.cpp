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

#ifdef __WIN32__
DltSettingsManager::DltSettingsManager()
{
    settings = new QSettings(QApplication::applicationDirPath()+"/config.ini", QSettings::IniFormat);
}
#else
DltSettingsManager::DltSettingsManager()
{
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "BMW","DLT Viewer");
}
#endif

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
