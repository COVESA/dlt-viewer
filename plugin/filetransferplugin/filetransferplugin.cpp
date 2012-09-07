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
 * \file filetransferplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>
#include "filetransferplugin.h"

FiletransferPlugin::FiletransferPlugin() {
    dltFile = 0;
    msgIndex = 0;
}

FiletransferPlugin::~FiletransferPlugin() {

}

QString FiletransferPlugin::name() {
    return QString("Filetransfer Plugin");
}

QString FiletransferPlugin::pluginVersion()
{
    return FILETRANSFER_PLUGIN_VERSION;
}

QString FiletransferPlugin::pluginInterfaceVersion()
{
    return PLUGIN_INTERFACE_VERSION;
}

QString FiletransferPlugin::description() {
    QString description("This plugin enables the user to get a list of embedded files in a dlt log and save these files to disk. ");
    description += ("For more informations about this plugin please have a look on the dlt filetransfer documentation (generate it with doxygen).");
    return description;
}

QString FiletransferPlugin::error() {
    return QString();
}

bool FiletransferPlugin::loadConfig(QString filename) {

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
             return false;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
          xml.readNext();

          if(xml.isStartElement())
          {
              if(xml.name() == QString("TAG_FLST"))
              {
                  config.setFlstTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLDA"))
              {
                  config.setFldaTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLFI"))
              {
                  config.setFlfiTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLER"))
              {
                  config.setFlerTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLAPPID"))
              {
                  config.setFlAppIdTag( xml.readElementText() );
              }
              if(xml.name() == QString("TAG_FLCTID"))
              {
                  config.setFlCtIdTag( xml.readElementText() );
              }
          }
    }
    if (xml.hasError()) {
        QMessageBox::warning(0, QString("XML Parser error"),
                             xml.errorString());
    }

    file.close();

    return true;
}

bool FiletransferPlugin::saveConfig(QString /*filename*/) {

    return true;
}

QStringList FiletransferPlugin::infoConfig() {

    QStringList list;

    list.append("TAG_FLAPPID: "+ config.getFlAppIdTag());
    list.append("TAG_FLCTID: "+ config.getFlCtIdTag());
    list.append("TAG_FLST: "+ config.getFlstTag());
    list.append("TAG_FLDA: "+ config.getFldaTag());
    list.append("TAG_FLFI: "+ config.getFlfiTag());
    list.append("TAG_FLER: "+ config.getFlerTag());

    return list;
}

QWidget* FiletransferPlugin::initViewer() {
    form = new Form();

    return form;
}

bool FiletransferPlugin::initFile(QDltFile *file) {
    dltFile = file;
    msgIndex = 0;
    form->getTreeWidget()->clear();
    form->clearSelectedFiles();
    updateFile();
    return true;
}

void FiletransferPlugin::updateFile() {
    QDltMsg msg;
    QDltArgument msgFirstArgument;
    QDltArgument msgLastArgument;

    if(!dltFile)
        return;

    for(;msgIndex<dltFile->size();msgIndex++)
    {
        if (!dltFile->getMsg(msgIndex, msg))
            break;


        if(config.getFlAppIdTag().compare(msg.getApid()) != 0 || config.getFlCtIdTag().compare(msg.getCtid()) != 0)
                 continue;

        if(!msg.getArgument(PROTOCOL_ALL_STARTFLAG,msgFirstArgument))
            continue;

        if(msgFirstArgument.toString().compare(config.getFlstTag()) == 0 )
        {
            msg.getArgument(PROTOCOL_FLST_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFlstTag()) == 0)
            {
                doFLST(&msg);
            }
            continue;
        }
        if(msgFirstArgument.toString().compare(config.getFldaTag()) == 0 ) {
            msg.getArgument(PROTOCOL_FLDA_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFldaTag()) == 0)
            {
                doFLDA(msgIndex,&msg);
            }
            continue;
        }
        if(msgFirstArgument.toString().compare(config.getFlfiTag()) == 0 ) {
            msg.getArgument(PROTOCOL_FLFI_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFlfiTag()) == 0)
            {
                doFLFI(&msg);
            }
            continue;
        }
        if(msgFirstArgument.toString().compare(config.getFlfiTag()) == 0 ) {
            msg.getArgument(PROTOCOL_FLIF_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFlfiTag()) == 0)
            {
                doFLIF(&msg);
            }
            continue;
        }
        if (msgFirstArgument.toString().compare(config.getFlerTag()) == 0 ) {
            msg.getArgument(PROTOCOL_FLER_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFlerTag()) == 0)
            {
                doFLER(&msg);
            }
            continue;
        }
    }
}

void FiletransferPlugin::selectedIdxMsg(int index) {
    if(!dltFile)
        return;
}

void FiletransferPlugin::doFLST(QDltMsg *msg){

    QDltArgument argument;
    msg->getArgument(PROTOCOL_FLST_FILEID,argument);

    File *file = new File(dltFile,0);
    file->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    file->setCheckState(COLUMN_CHECK, Qt::Unchecked);

    msg->getArgument(PROTOCOL_FLST_FILEID,argument);
    file->setFileSerialNumber(argument.toString());

    msg->getArgument(PROTOCOL_FLST_FILENAME,argument);
    file->setFilename(argument.toString());

    msg->getArgument(PROTOCOL_FLST_FILEDATE,argument);
    file->setFileCreationDate(argument.toString());

    msg->getArgument(PROTOCOL_FLST_SIZE,argument);
    file->setSizeInBytes(argument.toString());

    msg->getArgument(PROTOCOL_FLST_PACKAGES,argument);
    file->setPackages(argument.toString());

    msg->getArgument(PROTOCOL_FLST_BUFFERSIZE,argument);
    file->setBuffersize(argument.toString());

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(file->getFileSerialNumber(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);

    if(result.isEmpty()){
        form->getTreeWidget()->addTopLevelItem(file);
    }
    else
    {
      int index = form->getTreeWidget()->indexOfTopLevelItem(result.at(0));
      form->getTreeWidget()->takeTopLevelItem(index);
      form->getTreeWidget()->addTopLevelItem(file);
    }

}

void FiletransferPlugin::doFLDA(int index,QDltMsg *msg){
    QDltArgument argument;
    msg->getArgument(PROTOCOL_FLDA_FILEID,argument);

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(argument.toString(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);

    if(result.isEmpty())
    {
        //Transfer for this file started before sending FLST
    }
    else
    {
        File *file = (File*)result.at(0);
        if(!file->isComplete())
        {
            QDltArgument packageNumber;
            msg->getArgument(PROTOCOL_FLDA_PACKAGENR,packageNumber);

            file->setQFileIndexForPackage(packageNumber.toString(),index);
        }
    }
}

void FiletransferPlugin::doFLFI(QDltMsg *msg){
    QDltArgument argument;
    msg->getArgument(PROTOCOL_FLFI_FILEID,argument);

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(argument.toString(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);

    if(result.isEmpty())
    {
        //Transfer for this file started before sending FLST
    }
    else
    {
        File *file = (File*)result.at(0);
        if(file->isComplete())
        {
            file->setComplete();
        }
    }
}

void FiletransferPlugin::doFLIF(QDltMsg *msg){

}

void FiletransferPlugin::doFLER(QDltMsg *msg){
    QDltArgument filename;
    msg->getArgument(PROTOCOL_FLER_FILENAME,filename);
    QDltArgument errorCode1;
    msg->getArgument(PROTOCOL_FLER_ERRCODE1,errorCode1);
    QDltArgument errorCode2;
    msg->getArgument(PROTOCOL_FLER_ERRCODE2,errorCode2);

    File *file= new File(0);

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(filename.toString(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILENAME);

    if(result.isEmpty()){
       form->getTreeWidget()->addTopLevelItem(file);
    }
    else
    {
       file = (File*)result.at(0);
       int index = form->getTreeWidget()->indexOfTopLevelItem(result.at(0));
       form->getTreeWidget()->takeTopLevelItem(index);
       form->getTreeWidget()->addTopLevelItem(file);
    }

    file->errorHappens(filename.toString(),errorCode1.toString(),errorCode2.toString(),msg->getTimeString());
    file->setFlags(Qt::NoItemFlags );
}

Q_EXPORT_PLUGIN2(filetransferplugin, FiletransferPlugin);
