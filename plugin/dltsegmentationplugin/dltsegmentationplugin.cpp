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
 * \file dltsegmentationplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>

#include "dltsegmentationplugin.h"

DltSegmentationPlugin::DltSegmentationPlugin()
{
}

DltSegmentationPlugin::~DltSegmentationPlugin()
{

}

QString DltSegmentationPlugin::name()
{
    return QString("DLT Segmentation Plugin");
}

QString DltSegmentationPlugin::pluginVersion(){
    return DLT_SEGMENTATION_PLUGIN_VERSION;
}

QString DltSegmentationPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DltSegmentationPlugin::description()
{
    return QString();
}

QString DltSegmentationPlugin::error()
{
    return QString();
}

bool DltSegmentationPlugin::loadConfig(QString /* filename */ )
{
    return true;
}

bool DltSegmentationPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DltSegmentationPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DltSegmentationPlugin::initViewer()
{
    form = new DltSegmantationPlugin::Form();
    return form;
}

void DltSegmentationPlugin::selectedIdxMsg(int index, QDltMsg &/*msg*/) {
    if(!dltFile)
        return;
}

void DltSegmentationPlugin::selectedIdxMsgDecoded(int , QDltMsg &/*msg*/){

}

void DltSegmentationPlugin::initFileStart(QDltFile *file){
if(nullptr == file)
        return;

    dltFile = file;

    segmentedMessages.clear();
}

void DltSegmentationPlugin::initMsg(int index, QDltMsg &msg){
    if(nullptr == dltFile)
         return;

    if(msg.getWithSegementation())
    {
        // add msg to
        segmentedMessages.append(index);
    }

}

void DltSegmentationPlugin::initMsgDecoded(int , QDltMsg &){
    if(nullptr == dltFile)
         return;
}

void DltSegmentationPlugin::initFileFinish(){
   if(nullptr == dltFile)
        return;
}

void DltSegmentationPlugin::updateFileStart(){
    if(nullptr == dltFile)
         return;
}

void DltSegmentationPlugin::updateMsg(int index, QDltMsg &msg){
    if(!dltFile)
        return;

    if(msg.getWithSegementation())
    {
        // add msg to
        segmentedMessages.append(index);
    }
}

void DltSegmentationPlugin::updateMsgDecoded(int index, QDltMsg &){
    if(nullptr == dltFile)
         return;
}

void DltSegmentationPlugin::updateFileFinish(){
   if(nullptr == dltFile)
        return;
}

bool DltSegmentationPlugin::isMsg(QDltMsg & msg, int triggeredByUser)
{
    Q_UNUSED(msg);
    Q_UNUSED(triggeredByUser);

    if(msg.getWithSegementation())
    {
        if(msg.getSegmentationFrameType()==2)
        {
            // last Frame
            return true;
        }
    }

    return false;
}

bool DltSegmentationPlugin::decodeMsg(QDltMsg &msg, int triggeredByUser)
{
    Q_UNUSED(triggeredByUser);

    if(nullptr == dltFile)
         return false;

    if(msg.getWithSegementation())
    {
        if(msg.getSegmentationFrameType()==2)
        {
            // find position in array
            int index = msg.getIndex();
            QString appId = msg.getApid();
            QString ctxId = msg.getCtid();
            int lastIndex = segmentedMessages.indexOf(index);
            QByteArray payload = msg.getPayload();
            int consecutiveFrame=-1;

            // go back now to find chunks before with same app id and ctx id and descending consecutive frame
            for(int num=lastIndex-1;num>=0;num--)
            {
                QDltMsg msgChunk;
                dltFile->getMsg(segmentedMessages[num],msgChunk);
                if(msgChunk.getApid()==appId && msgChunk.getCtid()==ctxId)
                {
                    if(msgChunk.getSegmentationFrameType()==1)
                    {
                        payload.insert(0,msgChunk.getPayload());
                        if(consecutiveFrame==-1)
                        {
                            consecutiveFrame = msgChunk.getSegmentationConsecutiveFrame();
                        }
                        else if((consecutiveFrame-1) == msgChunk.getSegmentationConsecutiveFrame())
                        {
                            consecutiveFrame = msgChunk.getSegmentationConsecutiveFrame();
                        }
                        else
                        {
                            // wrong order of chunks
                            return false;
                        }
                    }
                    else if(msgChunk.getSegmentationFrameType()==0 && consecutiveFrame== 0)
                    {
                        payload.insert(0,msgChunk.getPayload());

                        if(msgChunk.getSegmentationTotalLength()==payload.length())
                        {
                            msg.setPayload(payload);
                            msg.parseArguments();
                            return true;
                        }
                        else
                        {
                            // wrong length of payload
                            return false;
                        }
                    }

                }
            }

            return false;
        }
    }

    return false;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(dltsegmentationplugin, DltSegmentationPlugin);
#endif

