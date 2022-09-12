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
 * \file dltlogstorageconfigcreatorplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>

#include "dltlogstorageconfigcreatorplugin.h"

DltLogstorageConfigCreatorPlugin::DltLogstorageConfigCreatorPlugin() {

}

DltLogstorageConfigCreatorPlugin::~DltLogstorageConfigCreatorPlugin() {

}

/* Dlt Viewer plugin interface */

QString DltLogstorageConfigCreatorPlugin::name() {
    return QString("DLT Logstorage Config Creator Plugin");
}

QString DltLogstorageConfigCreatorPlugin::pluginVersion(){
    return DLT_LOGSTORAGE_CONFIG_CREATOR_PLUGIN_VERSION;
}

QString DltLogstorageConfigCreatorPlugin::pluginInterfaceVersion() {
    return PLUGIN_INTERFACE_VERSION;
}

QString DltLogstorageConfigCreatorPlugin::description() {
    return QString("DLT Viewer plugin to create and modify DLT Offline Logstorage configuration files");
}

QString DltLogstorageConfigCreatorPlugin::error() {
    return QString();
}

bool DltLogstorageConfigCreatorPlugin::loadConfig(QString /*filename*/) {
    return true;
}

bool DltLogstorageConfigCreatorPlugin::saveConfig(QString /*filename*/) {
    return true;
}

QStringList DltLogstorageConfigCreatorPlugin::infoConfig() {
    return QStringList();
}

QWidget* DltLogstorageConfigCreatorPlugin::initViewer() {
    ui = new LogstorageConfigCreatorForm();
    return ui;
}

void DltLogstorageConfigCreatorPlugin::initFileStart(QDltFile *file) {
    (void) file;
}

void DltLogstorageConfigCreatorPlugin::initFileFinish() {

}

void DltLogstorageConfigCreatorPlugin::initMsg(int index, QDltMsg &msg) {
    (void) index; /* not used */
    (void) msg; /* not used */
}

void DltLogstorageConfigCreatorPlugin::initMsgDecoded(int index, QDltMsg &msg) {
    (void) index; /* not used */
    (void) msg; /* not used */
}

void DltLogstorageConfigCreatorPlugin::updateFileStart() {

}

void DltLogstorageConfigCreatorPlugin::updateMsg(int index, QDltMsg &msg) {
    (void) index; /* not used */
    (void) msg; /* not used */
}

void DltLogstorageConfigCreatorPlugin::updateMsgDecoded(int index, QDltMsg &msg) {
    (void) index; /* not used */
    (void) msg; /* not used */
}

void DltLogstorageConfigCreatorPlugin::updateFileFinish() {

}

void DltLogstorageConfigCreatorPlugin::selectedIdxMsg(int index, QDltMsg &msg) {
    (void) index; /* not used */
    (void) msg; /* not used */
}

void DltLogstorageConfigCreatorPlugin::selectedIdxMsgDecoded(int index, QDltMsg &msg) {
    (void) index; /* not used */
    (void) msg; /* not used */
}

#ifndef QT5
Q_EXPORT_PLUGIN2(dltlogstorageconfigcreatorplugin, DltLogstorageConfigCreatorPlugin);
#endif // DLTLOGSTORAGECONFIGCREATORPLUGIN_H
