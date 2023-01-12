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
 * \file filetransferplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QtDebug>

#include "filetransferplugin.h"
#include "file.h"


FiletransferPlugin::FiletransferPlugin()
{
    form = NULL;
    dltFile = NULL;
}

FiletransferPlugin::~FiletransferPlugin()
{
}

QString FiletransferPlugin::name()
{
    return plugin_name_displayed;
}

QString FiletransferPlugin::pluginVersion()
{
    return FILETRANSFER_PLUGIN_VERSION;
}

QString FiletransferPlugin::pluginInterfaceVersion()
{
    return PLUGIN_INTERFACE_VERSION;
}

QString FiletransferPlugin::description()
{
    QString description("This plugin enables the user to get a list of embedded files in a dlt log and save these files to disk. ");
    description += ("For more informations about this plugin please have a look on the dlt filetransfer documentation (generate it with doxygen).");
    return description;
}

QString FiletransferPlugin::error()
{
    return errorText;
}

bool FiletransferPlugin::loadConfig(QString filename)
{
    if ( filename.length() <= 0 )
    {
        // no configuration file provided, return to default configuration
        config.setDefault();
        // return no error
        errorText = "";
        return true;
    }

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        errorText = "Can not load configuration File: ";
        errorText.append(filename);
        return false;
    }

    config.setDefault();
    ft_autosave = false;
    form->setAutoSave(config.getAutoSavePath(), ft_autosave);

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
              if(xml.name() == QString("TAG_FLCTID"))
              {
                  config.setFlCtIdTag( xml.readElementText() );
              }
              if(xml.name() == QString("AUTOSAVE"))
              {
                  config.setAutoSavePath( xml.readElementText() );
                  ft_autosave = true;
                  if (!QDir(config.getAutoSavePath()).exists())
                  {
                    if ( false == QDir().mkpath(config.getAutoSavePath()) )
                    {
                     if ( dltControl->silentmode == true )
                      {
                      qDebug() << "ERROR creating autosave folder" << config.getAutoSavePath();
                      }
                     else
                      {
                       QMessageBox::warning(0, QString("ERROR creating autosave folder"), config.getAutoSavePath());
                      }
                    }
                  }
                form->setAutoSave(config.getAutoSavePath(), true);
              }

          }
    }
    if (xml.hasError())
    {
      if ( dltControl->silentmode == true )
      {
       qDebug() << plugin_name_displayed << QString("XML Parser error %1 at line %2").arg(xml.errorString()).arg(xml.lineNumber());
      }
      else
      {
        QMessageBox::warning(0, QString("XML Parser error"), xml.errorString());
      }
    }

    file.close();

    return true;
}

bool FiletransferPlugin::saveConfig(QString /*filename*/)
{

    return true;
}

QStringList FiletransferPlugin::infoConfig()
{
    QStringList list;

    list.append("TAG_FLCTID: "+ config.getFlCtIdTag());
    list.append("TAG_FLST: "+ config.getFlstTag());
    list.append("TAG_FLDA: "+ config.getFldaTag());
    list.append("TAG_FLFI: "+ config.getFlfiTag());
    list.append("TAG_FLER: "+ config.getFlerTag());
    if ( true == ft_autosave )
     {
      list.append("Autosave: "+ config.getAutoSavePath());
     }

    return list;
}

QWidget* FiletransferPlugin::initViewer()
{
    form = new FileTransferPlugin::Form();
    return form;
}

void FiletransferPlugin::selectedIdxMsg(int , QDltMsg &)
{
//empty. Implemented because derived plugin interface functions are virtual.
}

void FiletransferPlugin::selectedIdxMsgDecoded(int , QDltMsg &)
{
//empty. Implemented because derived plugin interface functions are virtual.
}

void FiletransferPlugin::initFileStart(QDltFile *file)
{
	if (plugin_is_active == false )
	{
    qDebug() << "Activate plugin" << plugin_name_displayed <<  FILETRANSFER_PLUGIN_VERSION;
    plugin_is_active = true;
    }
    dltFile = file;
    form->getTreeWidget()->clear();
    form->clearSelectedFiles();
}

void FiletransferPlugin::initMsg(int index, QDltMsg &msg)
{
   // this function is called when live trace is acvtive
   updateFiletransfer(index,msg);
}

void FiletransferPlugin::initMsgDecoded(int , QDltMsg &)
{
//empty. Implemented because derived plugin interface functions are virtual.
}

void FiletransferPlugin::initFileFinish()
{
//empty. Implemented because derived plugin interface functions are virtual.
}

void FiletransferPlugin::updateFileStart()
{
//empty. Implemented because derived plugin interface functions are virtual.
}

void FiletransferPlugin::updateMsg(int index, QDltMsg &msg)
{
  // this function is called when a dlt file is read
    updateFiletransfer(index,msg);
  // end of void function
}

void FiletransferPlugin::updateMsgDecoded(int , QDltMsg &)
{//empty. Implemented because derived plugin interface functions are virtual.
}

void FiletransferPlugin::updateFileFinish()
{
}

void FiletransferPlugin::updateFiletransfer(int index, QDltMsg &msg)
{
    QDltArgument msgFirstArgument;
    QDltArgument msgLastArgument;

    if(NULL == dltFile)
    {
        //qDebug()<< "dltfile object does not exist";
        return;
    }

    if(msg.getType() != QDltMsg::DltTypeLog)
    {
        return;
    }


    if(config.getFlCtIdTag().compare(msg.getCtid()) != 0)
    {
        // message is not of CTID defined to indicate file transfer
        return;
    }

    if(!msg.getArgument(PROTOCOL_ALL_STARTFLAG,msgFirstArgument))
    {
        return;
    }

    // so we have a valid file transfer packet and now we look for the tag contained in the message

    if(msgFirstArgument.toString().compare(config.getFldaTag()) == 0 ) // Filetransfer Update
    {
            msg.getArgument(PROTOCOL_FLDA_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFldaTag()) == 0)
            {
                doFLDA(index,&msg);
            }
            return;
     }

    if(msgFirstArgument.toString().compare(config.getFlstTag()) == 0 ) // Filetransfer Start
    {
            msg.getArgument(PROTOCOL_FLST_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFlstTag()) == 0)
            {
                doFLST(&msg);
            }
            return;
    }

    if(msgFirstArgument.toString().compare(config.getFlfiTag()) == 0 ) // Filetransfer Stop
    {
            msg.getArgument(PROTOCOL_FLFI_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFlfiTag()) == 0)
            {
                doFLDA(index,&msg);
            }
            return;
    }

    if (msgFirstArgument.toString().compare(config.getFlerTag()) == 0 )
    {
            msg.getArgument(PROTOCOL_FLER_ENDFLAG,msgLastArgument);
            if(msgLastArgument.toString().compare(config.getFlerTag()) == 0)
            {
                doFLER(&msg);
            }
            return;
    }

    // end of void function
}


void FiletransferPlugin::doFLST(QDltMsg *msg)
{

    QDltArgument argument;
    msg->getArgument(PROTOCOL_FLST_FILEID,argument);

    File *file = new File(dltFile,0);

    if ( file == NULL )
    {
      qDebug() << "Unable to create file object in "  << __LINE__ << __FILE__;
      return;
    }

    // set the attributes of the dynamic widget
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

    emit(form->additem_signal(file));

  return;
}

void FiletransferPlugin::doFLDA(int index,QDltMsg *msg)
{
    QDltArgument argument;
    QDltArgument packageNumber;
    msg->getArgument(PROTOCOL_FLDA_FILEID,argument);
    msg->getArgument(PROTOCOL_FLDA_PACKAGENR,packageNumber);

    emit(form->handleupdate_signal(argument.toString(),packageNumber.toString(), index ));
  return;
}

void FiletransferPlugin::doFLIF(QDltMsg *msg)
{
    Q_UNUSED(msg);

//empty.
//not implemented yet. Would handle extended file information: file serialnumber, name ,size, creation date, number of packages
}

void FiletransferPlugin::doFLER(QDltMsg *msg)
{
    QDltArgument filename;
    msg->getArgument(PROTOCOL_FLER_FILENAME,filename);
    QDltArgument errorCode1;
    msg->getArgument(PROTOCOL_FLER_ERRCODE1,errorCode1);
    QDltArgument errorCode2;
    msg->getArgument(PROTOCOL_FLER_ERRCODE2,errorCode2);

    emit(form->handle_errorsignal(filename.toString(),errorCode1.toString(),errorCode2.toString(),msg->getTimeString()));
}

bool FiletransferPlugin::command(QString command, QList<QString> params)
{
    if(!dltFile)
     {
         qDebug()<< "FiletransferPlugin not active !";
         return false;
     }

    if(command.compare("export", Qt::CaseInsensitive) == 0)
    {
        if(params.length() != 1)
        {
            errorText = "Need one parameter, path to save to.";
            return false;
        }
        QString exp_path = params.at(0);
        QChar backslash('\"');

        if ( exp_path.at(exp_path.length()-1) == backslash )
        {
            exp_path.chop(1);//remove the last \", which would cause problems under windows
        }

        QDir extract_dir(QDir::fromNativeSeparators ( exp_path ));

        if(false == extract_dir.exists())
        {
          if (!extract_dir.mkpath("."))
            {
              errorText = "Failed to create directory " + params.at(0);
              return false;
            }
        }
        else
        {
            qDebug() << "Created" << params.at(0);
        }

        return exportAll(extract_dir);
    }
    errorText = "Unknown command " + command;
    return false;
}

bool FiletransferPlugin::exportAll(QDir extract_dir)
{
   bool ret = true;
   QApplication::processEvents();
   emit(form->export_signal(extract_dir,&errorText, &ret));
   return ret;
}

/* Control Plugin methods */

// these are only needed to get information about silent mode via
// dltcontrol
bool FiletransferPlugin::initControl(QDltControl *control)
{
    dltControl = control;
    return true;
}


bool FiletransferPlugin::initConnections(QStringList list)
{
    Q_UNUSED(list);
    return true;
}

bool FiletransferPlugin::controlMsg(int , QDltMsg &)
{
    return true;
}

bool FiletransferPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname)
{
    Q_UNUSED(index);
    Q_UNUSED(connectionState);
    Q_UNUSED(hostname);

    return true;
}

bool FiletransferPlugin::autoscrollStateChanged(bool enabled)
{
    Q_UNUSED(enabled);
    return true;
}

void FiletransferPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
    Q_UNUSED(pMessageDecoder);
}

void FiletransferPlugin::initMainTableView(QTableView* pTableView)
{
    Q_UNUSED(pTableView);
}

void FiletransferPlugin::configurationChanged()
{}


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(filetransferplugin, FiletransferPlugin)
#endif
