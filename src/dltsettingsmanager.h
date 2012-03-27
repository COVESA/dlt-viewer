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
 * \author Lassi Marttala <Lassi.LM.Marttala@parner.bmw.de>
 *
 * \file dltsettingsmanager.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef DLTSETTINGSMANAGER_H
#define DLTSETTINGSMANAGER_H

#include <qsettings.h>

class DltSettingsManager
{
// Singleton pattern
public:
    static DltSettingsManager* instance();
    static void close();

private:
    DltSettingsManager();
    ~DltSettingsManager();
    static DltSettingsManager *m_instance;
    QSettings *settings;

// QSettings delegates
public:
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void clear();
    QString fileName() const;

};

#endif // DLTSETTINGSMANAGER_H
