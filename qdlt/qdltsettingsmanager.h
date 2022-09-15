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
 * \author Lassi Marttala <Lassi.LM.Marttala@parner.bmw.de>
 *
 * \file dltsettingsmanager.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLTSETTINGSMANAGER_H
#define QDLTSETTINGSMANAGER_H

#include <QColor>
#include <qsettings.h>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "export_rules.h"

#define DEFAULT_REFRESH_RATE 20

class QDLT_EXPORT QDltSettingsManager
{
// Singleton pattern
public:
    static QDltSettingsManager* getInstance();
    static void close();

private:
    QDltSettingsManager();
    ~QDltSettingsManager();
    static QDltSettingsManager *m_instance;
    QSettings *settings;

// QSettings delegates
public:
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void clear();
    QString fileName() const;

    void writeSettings();
    void readSettings();

    void writeSettingsLocal(QXmlStreamWriter &xml);
    void readSettingsLocal(QXmlStreamReader &xml);

    int     tempUseSystem; // local setting
    QString tempSystemPath; // local setting
    int     tempUseOwn; // local setting
    QString tempOwnPath; // local setting
    int     tempCloseWithoutAsking; // local setting
    int     tempSaveOnClear; // local setting

    int defaultLogFile; // local setting
    QString defaultLogFileName; // local setting
    int defaultProjectFile; // local setting
    QString defaultProjectFileName; // local setting
    int pluginsPath; // local setting
    QString pluginsPathName; // local setting
    int defaultFilterPath; // local setting
    QString defaultFilterPathName; // local setting
    int pluginsAutoloadPath; // local setting
    QString pluginsAutoloadPathName; // local setting
    int filterCache; // local setting
    QByteArray geometry; // local setting
    QByteArray windowState; // local setting
    int RefreshRate; // local setting
    int StartupMinimized; // local settings

    QColor markercolor; // local and project setting
    int autoConnect; // project and local setting
    int autoScroll; // project and local setting
    int autoMarkFatalError; // local and local setting
    int autoMarkWarn; // project and local setting
    int autoMarkMarker; // project and local setting
    int writeControl; // project and local setting
    int updateContextLoadingFile; // project and local setting
    int updateContextsUnregister; // project and local setting
    int loggingOnlyMode; // project and local setting
    int splitlogfile; // local and project setting
    float fmaxFileSizeMB; // local and project setting
    int appendDateTime; // local and project setting

    int fontSize; // project and local setting
    int sectionSize; // project and local setting
    QString fontName; // project and local setting
    int showIndex; // project and local setting
    int showTime; // project and local setting
    int showTimestamp; // project and local setting
    int showCount; // project and local setting
    int showEcuId; // project and local setting
    int showApId; // project and local setting
    int showApIdDesc; // project and local setting
    int showCtId; // project and local setting
    int showCtIdDesc; // project and local setting
    int showSessionId; // project and local setting
    int showSessionName; // project and local setting
    int showType; // project and local setting
    int showSubtype; // project and local setting
    int showMode; // project and local setting
    int showNoar; // project and local setting
    int showPayload; // project and local setting
    int showArguments; // project and local setting
    int showMsgId; // project and local setting

    QString msgIdFormat; // project and local setting
    int automaticTimeSettings; // project and local setting
    int automaticTimezoneFromDlt; // project and local setting
    qlonglong utcOffset; // project and local setting
    int dst; // project and local setting

    QStringList pluginExecutionPrio; //local setting
};

#endif // QDLTSETTINGSMANAGER_H
