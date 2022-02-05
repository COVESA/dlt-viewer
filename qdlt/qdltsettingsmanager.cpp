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

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QDateTime>
#include <QStandardPaths>

#include "version.h"

#include "qdlt.h"

#if (WIN32)
 #define TZSET _tzset()
 #define TIMEZONE _timezone
 #define DAYLIGHT _daylight
#else
 #define TZSET tzset()
 #define TIMEZONE timezone
 #define DAYLIGHT daylight
#endif

QDltSettingsManager *QDltSettingsManager::m_instance = NULL;

QDltSettingsManager *QDltSettingsManager::getInstance()
{
    if(!m_instance)
        m_instance = new QDltSettingsManager();
    return m_instance;
}

void QDltSettingsManager::close()
{
    if(m_instance)
        delete m_instance;
    m_instance = NULL;
}

QDltSettingsManager::QDltSettingsManager()
{
    /* check if directory for configuration exists */
    QDir dir(QDir::homePath()+"/.dlt/config");
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

QDltSettingsManager::~QDltSettingsManager()
{
    delete settings;
}

void QDltSettingsManager::setValue(const QString &key, const QVariant &value)
{
    settings->setValue(key, value);
}

QVariant QDltSettingsManager::value(const QString &key, const QVariant &defaultValue) const
{
    return settings->value(key, defaultValue);
}

void QDltSettingsManager::clear()
{
    settings->clear();
}

QString QDltSettingsManager::fileName() const
{
    return settings->fileName();
}

void QDltSettingsManager::writeSettingsLocal(QXmlStreamWriter &xml)
{
    /* Write project settings */
    xml.writeStartElement("settings");
        xml.writeStartElement("table");
            xml.writeTextElement("fontSize",QString("%1").arg(fontSize));
            xml.writeTextElement("sectionSize",QString("%1").arg(sectionSize));
            xml.writeTextElement("fontName",fontName);
            xml.writeTextElement("automaticTimeSettings",QString("%1").arg(automaticTimeSettings));
            xml.writeTextElement("automaticTimezoneFromDlt",QString("%1").arg(automaticTimezoneFromDlt));
            xml.writeTextElement("utcOffset",QString("%1").arg(utcOffset));
            xml.writeTextElement("dst",QString("%1").arg(dst));
            xml.writeTextElement("showIndex",QString("%1").arg(showIndex));
            xml.writeTextElement("showTime",QString("%1").arg(showTime));
            xml.writeTextElement("showTimestamp",QString("%1").arg(showTimestamp));
            xml.writeTextElement("showCount",QString("%1").arg(showCount));
            xml.writeTextElement("showEcuId",QString("%1").arg(showEcuId));
            xml.writeTextElement("showApId",QString("%1").arg(showApId));
            xml.writeTextElement("showApIdDesc",QString("%1").arg(showApIdDesc));
            xml.writeTextElement("showCtId",QString("%1").arg(showCtId));
            xml.writeTextElement("showCtIdDesc",QString("%1").arg(showCtIdDesc));
            xml.writeTextElement("showType",QString("%1").arg(showType));
            xml.writeTextElement("showSubtype",QString("%1").arg(showSubtype));
            xml.writeTextElement("showMode",QString("%1").arg(showMode));
            xml.writeTextElement("showNoar",QString("%1").arg(showNoar));
            xml.writeTextElement("showPayload",QString("%1").arg(showPayload));
            xml.writeTextElement("showArguments",QString("%1").arg(showArguments));
            xml.writeTextElement("showMsgId",QString("%1").arg(showMsgId));
            xml.writeTextElement("markercolor",QString("%1").arg(markercolor.name()));
        xml.writeEndElement(); // table

        xml.writeStartElement("other");
            xml.writeTextElement("autoConnect",QString("%1").arg(autoConnect));
            xml.writeTextElement("autoScroll",QString("%1").arg(autoScroll));
            xml.writeTextElement("autoMarkFatalError",QString("%1").arg(autoMarkFatalError));
            xml.writeTextElement("autoMarkWarn",QString("%1").arg(autoMarkWarn));
            xml.writeTextElement("autoMarkMarker",QString("%1").arg(autoMarkMarker));
            xml.writeTextElement("writeControl",QString("%1").arg(writeControl));
            xml.writeTextElement("updateContextLoadingFile",QString("%1").arg(updateContextLoadingFile));
            xml.writeTextElement("updateContextsUnregister",QString("%1").arg(updateContextsUnregister));
            xml.writeTextElement("loggingOnlyMode",QString("%1").arg(loggingOnlyMode));
            xml.writeTextElement("splitlogfile",QString("%1").arg(splitlogfile));
            xml.writeTextElement("fmaxFileSizeMB",QString("%1").arg(fmaxFileSizeMB));
            xml.writeTextElement("appendDateTime",QString("%1").arg(appendDateTime));
            xml.writeTextElement("msgIdFormat",QString("%1").arg(msgIdFormat));
        xml.writeEndElement(); // other
    xml.writeEndElement(); // settings

}

void QDltSettingsManager::writeSettings()
{
    settings->setValue("geometry", geometry);
    settings->setValue("windowState", windowState);
    settings->setValue("RefreshRate",RefreshRate);
    settings->setValue("StartUpMinimized",StartupMinimized);

    /* Temporary directory */
    settings->setValue("tempdir/tempUseSystem", tempUseSystem);
    settings->setValue("tempdir/tempSystemPath", tempSystemPath);
    settings->setValue("tempdir/tempUseOwn", tempUseOwn);
    settings->setValue("tempdir/tempOwnPath", tempOwnPath);
    settings->setValue("tempdir/tempCloseWithoutAsking", tempCloseWithoutAsking);
    settings->setValue("tempdir/tempSaveOnClear", tempSaveOnClear);

    /* startup */
    settings->setValue("startup/defaultProjectFile",defaultProjectFile);
    settings->setValue("startup/defaultProjectFileName",defaultProjectFileName);
    settings->setValue("startup/defaultLogFile",defaultLogFile);
    settings->setValue("startup/defaultLogFileName",defaultLogFileName);
    settings->setValue("startup/pluginsPath",pluginsPath);
    settings->setValue("startup/pluginsPathName",pluginsPathName);
    settings->setValue("startup/defaultFilterPath",defaultFilterPath);
    settings->setValue("startup/defaultFilterPathName",defaultFilterPathName);
    settings->setValue("startup/pluginsAutoloadPath",pluginsAutoloadPath);
    settings->setValue("startup/pluginsAutoloadPathName",pluginsAutoloadPathName);
    settings->setValue("startup/filterCache",filterCache);
    settings->setValue("startup/autoConnect",autoConnect);
    settings->setValue("startup/autoScroll",autoScroll);
    settings->setValue("startup/autoMarkFatalError",autoMarkFatalError);
    settings->setValue("startup/autoMarkWarn",autoMarkWarn);
    settings->setValue("startup/autoMarkMarker",autoMarkMarker);
    settings->setValue("startup/loggingOnlyMode",loggingOnlyMode);
    settings->setValue("startup/splitfileyesno",splitlogfile);
    settings->setValue("startup/maxFileSizeMB",fmaxFileSizeMB);
    settings->setValue("startup/appendDateTime",appendDateTime);
    settings->setValue("startup/markercolor",markercolor.name());

    /* table */
    settings->setValue("startup/fontSize",fontSize);
    settings->setValue("startup/sectionSize",sectionSize);
    settings->setValue("startup/fontName",fontName);
    settings->setValue("startup/automaticTimeSettings",automaticTimeSettings);
    settings->setValue("startup/automaticTimezoneFromDlt",automaticTimezoneFromDlt);
    settings->setValue("startup/utcOffset",utcOffset);
    settings->setValue("startup/dst",dst);
    settings->setValue("startup/showIndex",showIndex);
    settings->setValue("startup/showTime",showTime);
    settings->setValue("startup/showTimestamp",showTimestamp);
    settings->setValue("startup/showCount",showCount);

    settings->setValue("startup/showEcuId",showEcuId);
    settings->setValue("startup/showApId",showApId);
    settings->setValue("startup/showApIdDesc",showApIdDesc);
    settings->setValue("startup/showCtId",showCtId);
    settings->setValue("startup/showCtIdDesc",showCtIdDesc);
    settings->setValue("startup/showSessionId",showSessionId);
    settings->setValue("startup/showSessionName",showSessionName);
    settings->setValue("startup/showType",showType);

    settings->setValue("startup/showSubtype",showSubtype);
    settings->setValue("startup/showMode",showMode);
    settings->setValue("startup/showNoar",showNoar);
    settings->setValue("startup/showPayload",showPayload);
    settings->setValue("startup/showArguments",showArguments);
    settings->setValue("startup/showMsgId",showMsgId);

    /* other */
    settings->setValue("startup/writeControl",writeControl);
    settings->setValue("startup/updateContextLoadingFile",updateContextLoadingFile);
    settings->setValue("startup/updateContextsUnregister",updateContextsUnregister);
    settings->setValue("startup/msgIdFormat",msgIdFormat);

    /* For settings integrity validation */
    settings->setValue("startup/versionMajor", PACKAGE_MAJOR_VERSION);
    settings->setValue("startup/versionMinor", PACKAGE_MINOR_VERSION);
    settings->setValue("startup/versionPatch", PACKAGE_PATCH_LEVEL);
}

void QDltSettingsManager::readSettingsLocal(QXmlStreamReader &xml)
{
    /* Project settings */
    if(xml.name() == QString("autoConnect"))
    {
        autoConnect = xml.readElementText().toInt();
    }
    if(xml.name() == QString("autoScroll"))
    {
        autoScroll = xml.readElementText().toInt();
    }
    if(xml.name() == QString("autoMarkFatalError"))
    {
        autoMarkFatalError = xml.readElementText().toInt();
    }
    if(xml.name() == QString("autoMarkWarn"))
    {
        autoMarkWarn = xml.readElementText().toInt();
    }
    if(xml.name() == QString("autoMarkMarker"))
    {
        autoMarkMarker = xml.readElementText().toInt();
    }
    if(xml.name() == QString("fontSize"))
    {
        fontSize = xml.readElementText().toInt();
    }
    if(xml.name() == QString("sectionSize"))
    {
        sectionSize = xml.readElementText().toInt();
    }
    if(xml.name() == QString("fontName"))
    {
        fontName = xml.readElementText();
    }
    if(xml.name() == QString("automaticTimeSettings"))
    {
        automaticTimeSettings = xml.readElementText().toInt();
    }
    if(xml.name() == QString("automaticTimezoneFromDlt"))
    {
        automaticTimezoneFromDlt = xml.readElementText().toInt();
    }
    if(xml.name() == QString("utcOffset"))
    {
        utcOffset = xml.readElementText().toLongLong();
    }
    if(xml.name() == QString("dst"))
    {
        dst = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showIndex"))
    {
        showIndex = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showTime"))
    {
        showTime = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showTimestamp"))
    {
        showTimestamp = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showCount"))
    {
        showCount = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showEcuId"))
    {
        showEcuId = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showApId"))
    {
        showApId = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showApIdDesc"))
    {
        showApIdDesc = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showCtId"))
    {
        showCtId = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showCtIdDesc"))
    {
        showCtIdDesc = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showSessionId"))
    {
        showSessionId = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showSessionName"))
    {
        showSessionName = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showType"))
    {
        showType = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showSubtype"))
    {
        showSubtype = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showMode"))
    {
        showMode = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showNoar"))
    {
        showNoar = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showPayload"))
    {
        showPayload = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showArguments"))
    {
        showArguments = xml.readElementText().toInt();
    }
    if(xml.name() == QString("loggingOnlyMode"))
    {
        loggingOnlyMode = xml.readElementText().toInt();
    }
    if(xml.name() == QString("markercolor"))
    {
        markercolor.setNamedColor(xml.readElementText());
    }
    if(xml.name() == QString("writeControl"))
    {
        writeControl = xml.readElementText().toInt();
    }
    if(xml.name() == QString("updateContextLoadingFile"))
    {
        updateContextLoadingFile = xml.readElementText().toInt();
    }
    if(xml.name() == QString("updateContextsUnregister"))
    {
        updateContextsUnregister = xml.readElementText().toInt();
    }
    if(xml.name() == QString("splitlogfile"))
    {
        splitlogfile = xml.readElementText().toInt();
    }
    if(xml.name() == QString("fmaxFileSizeMB"))
    {
        fmaxFileSizeMB = xml.readElementText().toFloat();
    }
    if(xml.name() == QString("appendDateTime"))
    {
        appendDateTime = xml.readElementText().toInt();
    }
    if(xml.name() == QString("showMsgId"))
    {
        showMsgId = xml.readElementText().toInt();
    }
    if(xml.name() == QString("msgIdFormat"))
    {
        msgIdFormat = xml.readElementText();
    }

}

/* read the settings from config.ini */
void QDltSettingsManager::readSettings()
{
    /* Temp file */
    tempUseSystem               = settings->value("tempdir/tempUseSystem", 1).toInt();
    tempSystemPath              = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    tempUseOwn                  = settings->value("tempdir/tempUseOwn", 0).toInt();
    tempOwnPath                 = settings->value("tempdir/tempOwnPath", QString("")).toString();
    tempCloseWithoutAsking      = settings->value("tempdir/tempCloseWithoutAsking", 0).toInt();
    tempSaveOnClear             = settings->value("tempdir/tempSaveOnClear", 1).toInt();

    geometry = settings->value("geometry",0).toByteArray();
    windowState = settings->value("windowState",0).toByteArray();

    RefreshRate = settings->value("RefreshRate",DEFAULT_REFRESH_RATE).toInt();
    StartupMinimized = settings->value("StartupMinimized",0).toInt();

    /* startup */
    defaultProjectFile = settings->value("startup/defaultProjectFile",0).toInt();
    defaultProjectFileName = settings->value("startup/defaultProjectFileName",QString("")).toString();
    defaultLogFile = settings->value("startup/defaultLogFile",0).toInt();
    defaultLogFileName = settings->value("startup/defaultLogFileName",QString("")).toString();
    pluginsPath = settings->value("startup/pluginsPath",0).toInt();
    pluginsPathName = settings->value("startup/pluginsPathName",QString("")).toString();
    defaultFilterPath = settings->value("startup/defaultFilterPath",1).toInt();
    defaultFilterPathName = settings->value("startup/defaultFilterPathName",QDir::homePath()+"/.dlt/filters").toString();
    pluginsAutoloadPath = settings->value("startup/pluginsAutoloadPath",0).toInt();
    pluginsAutoloadPathName = settings->value("startup/pluginsAutoloadPathName",QString("")).toString();
    filterCache = settings->value("startup/filterCache",1).toInt();
    autoConnect = settings->value("startup/autoConnect",0).toInt();
    autoScroll = settings->value("startup/autoScroll",1).toInt();
    autoMarkFatalError = settings->value("startup/autoMarkFatalError",0).toInt();
    autoMarkWarn = settings->value("startup/autoMarkWarn",0).toInt();
    autoMarkMarker = settings->value("startup/autoMarkMarker",1).toInt();
    loggingOnlyMode = settings->value("startup/loggingOnlyMode",0).toInt();
    splitlogfile = settings->value("startup/splitfileyesno",0).toInt();
    fmaxFileSizeMB = settings->value("startup/maxFileSizeMB",0).toFloat();
    appendDateTime = settings->value("startup/appendDateTime",0).toInt();
    markercolor.setNamedColor(settings->value("startup/markercolor","#aaaaaa").toString() );

    /* project table */
    fontSize = settings->value("startup/fontSize",8).toInt();
    sectionSize = settings->value("startup/sectionSize",16).toInt();
    fontName = settings->value("startup/fontName","Segoe UI,8").toString();
    automaticTimeSettings = settings->value("startup/automaticTimeSettings",1).toInt();
    automaticTimezoneFromDlt = settings->value("startup/automaticTimezoneFromDlt",1).toInt();

    utcOffset = settings->value("startup/utcOffset",QVariant((qlonglong)TIMEZONE * (-1) )).toLongLong();
    dst = settings->value("startup/dst", DAYLIGHT == 0 ? 0 : 1).toInt();

    showIndex = settings->value("startup/showIndex",1).toInt();
    showTime = settings->value("startup/showTime",1).toInt();
    showTimestamp = settings->value("startup/showTimestamp",1).toInt();
    showCount = settings->value("startup/showCount",0).toInt();

    showEcuId = settings->value("startup/showEcuId",1).toInt();
    showApId = settings->value("startup/showApId",1).toInt();
    showApIdDesc = settings->value("startup/showApIdDesc",0).toInt();
    showCtId = settings->value("startup/showCtId",1).toInt();
    showCtIdDesc = settings->value("startup/showCtIdDesc",0).toInt();
    showSessionId = settings->value("startup/showSessionId",0).toInt();
    showSessionName = settings->value("startup/showSessionName",0).toInt();
    showType = settings->value("startup/showType",1).toInt();

    showSubtype = settings->value("startup/showSubtype",0).toInt();

    showMode = settings->value("startup/showMode",0).toInt();
    showNoar = settings->value("startup/showNoar",0).toInt();
    showPayload = settings->value("startup/showPayload",1).toInt();
    showArguments = settings->value("startup/showArguments",0).toInt();
    showMsgId = settings->value("startup/showMsgId",0).toInt();
    /* other */
    writeControl = settings->value("startup/writeControl",1).toInt();
    updateContextLoadingFile = settings->value("startup/updateContextLoadingFile",1).toInt();
    updateContextsUnregister = settings->value("startup/updateContextsUnregister",0).toInt();
    msgIdFormat = settings->value("startup/msgIdFormat","[%08u]").toString();
}

