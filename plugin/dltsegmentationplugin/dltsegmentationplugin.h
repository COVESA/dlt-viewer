/**
 * @licence app begin@
 * Copyright (C) 2023  BMW AG
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
 * \file dltsegmentationplugin.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DLTSEGMENTATIONPLUGIN_H
#define DLTSEGMENTATIONPLUGIN_H

#include <QObject>
#include <QVector>
#include "plugininterface.h"
#include "form.h"

#define DLT_SEGMENTATION_PLUGIN_VERSION "1.0.0"

class DltSegmentationPlugin : public QObject, QDLTPluginInterface, QDLTPluginDecoderInterface, QDltPluginViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDLTPluginDecoderInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DltSegmentationPlugin")
#endif

public:
    DltSegmentationPlugin();
    ~DltSegmentationPlugin();

    /* QDLTPluginInterface interface */
    QString name();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString description();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginViewerInterface */
    QWidget* initViewer();
    void initFileStart(QDltFile *file);
    void initFileFinish();
    void initMsg(int index, QDltMsg &msg);
    void initMsgDecoded(int index, QDltMsg &msg);
    void updateFileStart();
    void updateMsg(int index, QDltMsg &msg);
    void updateMsgDecoded(int index, QDltMsg &msg);
    void updateFileFinish();
    void selectedIdxMsg(int index, QDltMsg &msg);
    void selectedIdxMsgDecoded(int index, QDltMsg &msg);

    /* QDltPluginDecoderInterface */
    bool isMsg(QDltMsg &msg, int triggeredByUser);
    bool decodeMsg(QDltMsg &msg, int triggeredByUser);

    /* internal variables */
    DltSegmantationPlugin::Form *form;

private:
    QDltFile *dltFile;
    QString errorText;

    QVector<int> segmentedMessages;
};

#endif // DLTSEGMENTATIONPLUGIN_H
