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
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011,2012
 *
 * \file speedplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>

#include "speedplugin.h"

SpeedPlugin::SpeedPlugin()
{
    form = NULL;
    dltFile = 0;
    msgIndex = 0;
}

SpeedPlugin::~SpeedPlugin()
{

}

QString SpeedPlugin::name()
{
    return QString("Speed Plugin");
}

QString SpeedPlugin::description()
{
    return QString();
}

QString SpeedPlugin::error()
{
    return QString();
}

bool SpeedPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool SpeedPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList SpeedPlugin::infoConfig()
{
    return QStringList();
}

QWidget* SpeedPlugin::initViewer()
{
    form = new Form();
    return form;
}

bool SpeedPlugin::initFile(QDltFile *file)
{
    dltFile = file;

    return true;
}

void SpeedPlugin::updateFile()
{
    QByteArray buffer;
    QDltMsg msg;
    QDltArgument argument;
    if(!dltFile)
        return;




   for(;msgIndex<dltFile->size();msgIndex++)
    {

       buffer =  dltFile->getMsg(msgIndex);

       if(buffer.isEmpty())
           break;

       msg.setMsg(buffer);

        if( (msg.getApid().compare("SPEE") == 0) && (msg.getCtid().compare("SIG") == 0))
        {
            if(msg.getArgument(1,argument)) {
                            form->setSpeedLCD(argument,msg.getTimestamp());
            }

        }
    }
}

void SpeedPlugin::selectedIdxMsg(int index)
{
    if(!dltFile)
        return;

}

Q_EXPORT_PLUGIN2(speedplugin, SpeedPlugin);
