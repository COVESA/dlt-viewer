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
 * \file dummyviewerplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>

#include "dummyviewerplugin.h"

DummyViewerPlugin::DummyViewerPlugin()
{
    dltFile = 0;
}

DummyViewerPlugin::~DummyViewerPlugin()
{

}

QString DummyViewerPlugin::name()
{
    return QString("Dummy Viewer Plugin");
}

QString DummyViewerPlugin::pluginVersion(){
    return DUMMY_VIEWER_PLUGIN_VERSION;
}

QString DummyViewerPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DummyViewerPlugin::description()
{
    return QString();
}

QString DummyViewerPlugin::error()
{
    return QString();
}

bool DummyViewerPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool DummyViewerPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DummyViewerPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DummyViewerPlugin::initViewer()
{
    form = new Form();
    return form;
}

bool DummyViewerPlugin::initFile(QDltFile *file)
{
    dltFile = file;

    form->setMessages(dltFile->size());

    counterMessages = dltFile->size();

    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;

    updateCounters(0,counterMessages-1);

    return true;
}

void DummyViewerPlugin::updateFile()
{
    if(!dltFile)
        return;

    updateCounters(counterMessages-1,dltFile->size()-1);

    counterMessages = dltFile->size();
}

void DummyViewerPlugin::selectedIdxMsg(int index)
{
    if(!dltFile)
        return;

    form->setSelectedMessage(index);
}

void DummyViewerPlugin::updateCounters(int start,int end)
{
    QByteArray data;
    QDltMsg msg;

    if(!dltFile)
        return;


    for(int num=start;num<=end;num++)
    {
        if(dltFile->getMsg(num,msg)==true)
        {
            if(msg.getMode() == QDltMsg::DltModeVerbose)
            {
                counterVerboseMessages++;
            }
            if(msg.getMode() == QDltMsg::DltModeNonVerbose)
            {
                counterNonVerboseMessages++;
            }
        }
    }


   form->setMessages(dltFile->size());
   form->setVerboseMessages(counterVerboseMessages);
   form->setNonVerboseMessages(counterNonVerboseMessages);

}

Q_EXPORT_PLUGIN2(dummyviewerplugin, DummyViewerPlugin);
