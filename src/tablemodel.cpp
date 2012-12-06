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
 * \file treemodel.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>
#include <qmessagebox.h>

#include "tablemodel.h"

char buffer[DLT_VIEWER_LIST_BUFFER_SIZE];

 TableModel::TableModel(const QString & /*data*/, QObject *parent)
     : QAbstractTableModel(parent)
 {
 }

 TableModel::~TableModel()
 {

 }

 int TableModel::columnCount(const QModelIndex & /*parent*/) const
 {
     return DLT_VIEWER_COLUMN_COUNT;
 }

 QVariant TableModel::data(const QModelIndex &index, int role) const
 {
     QDltMsg msg;
     QByteArray buf;

     if (!index.isValid())
         return QVariant();

     if (index.row() >= qfile->sizeFilter() && index.row()<0)
         return QVariant();

     if (role == Qt::DisplayRole)
     {
         /* get the message with the selected item id */
         if(!qfile->getMsg(qfile->getMsgFilterPos(index.row()), msg))
         {
             if(index.column() == cnIndex)
             {
                 return QString("%1").arg(qfile->getMsgFilterPos(index.row()));
             }
             else if(index.column() == cnPayload)
             {
                 return QString("!!CORRUPTED MESSAGE!!");
             }
             return QVariant();
         }
         for(int num = 0; num < project->plugin->topLevelItemCount (); num++)
         {
             PluginItem *item = (PluginItem*)project->plugin->topLevelItem(num);

             if(item->getMode() != item->ModeDisable && item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg,0))
             {
                 item->plugindecoderinterface->decodeMsg(msg,0);
                 break;
             }
         }

         switch(index.column())
         {
         case cnIndex:
             /* display index */
             return QString("%1").arg(qfile->getMsgFilterPos(index.row()));
         case cnTime:
             if( project->settings->automaticTimeSettings == 0 )
                return QString("%1.%2").arg(msg.getGmTimeWithOffsetString(project->settings->utcOffset,project->settings->dst)).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
             else
                return QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
         case cnTimeStamp:
             return QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
         case cnCounter:
             return QString("%1").arg(msg.getMessageCounter());
         case cnEcuId:
             return msg.getEcuid();
         case cnAppId:
             switch(project->settings->showApIdDesc){
             case 0:
                 return msg.getApid();
                 break;
             case 1:
                   for(int num = 0; num < project->ecu->topLevelItemCount (); num++)
                    {
                     EcuItem *ecuitem = (EcuItem*)project->ecu->topLevelItem(num);
                     for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
                     {
                         ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);
                         if(appitem->id == msg.getApid() && !appitem->description.isEmpty())
                         {
                            return appitem->description;
                         }
                     }
                    }
                   return QString("Apid: %1 (No description)").arg(msg.getApid());
                 break;
              default:
                 return msg.getApid();
             }
         case cnContextId:
             switch(project->settings->showCtIdDesc){
             case 0:
                 return msg.getCtid();
                 break;
             case 1:

                   for(int num = 0; num < project->ecu->topLevelItemCount (); num++)
                    {
                     EcuItem *ecuitem = (EcuItem*)project->ecu->topLevelItem(num);
                     for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
                     {
                         ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);
                         for(int numcontext = 0; numcontext < appitem->childCount(); numcontext++)
                         {
                             ContextItem * conitem = (ContextItem *) appitem->child(numcontext);

                             if(appitem->id == msg.getApid() && conitem->id == msg.getCtid()
                                     && !conitem->description.isEmpty())
                             {
                                return conitem->description;
                             }
                         }
                     }
                    }
                   return  QString("Ctid: %1 (No description)").arg(msg.getCtid());
                 break;
              default:
                 return msg.getCtid();
             }
         case cnType:
             return msg.getTypeString();
         case cnSubtype:
             return msg.getSubtypeString();
         case cnMode:
             return msg.getModeString();
         case cnArgCount:
             return QString("%1").arg(msg.getNumberOfArguments());
         case cnPayload:
             /* display payload */
             return msg.toStringPayload();
         }
     }

     if ( role == Qt::ForegroundRole ) {
         qfile->getMsg(qfile->getMsgFilterPos(index.row()), msg); // getMsg can be better optimized than the two single calls
         if(project->settings->autoMarkFatalError && !qfile->checkMarker(msg).isValid() && ( msg.getSubtypeString() == "error" || msg.getSubtypeString() == "fatal")  ){
            return QVariant(QBrush(QColor(255,255,255)));
         } else {
            return QVariant(QBrush(QColor(0,0,0)));
         }
     }

     if ( role == Qt::BackgroundRole ) {
         qfile->getMsg(qfile->getMsgFilterPos(index.row()), msg); // getMsg can be better optimized than the two single calls
         for(int num = 0; num < project->plugin->topLevelItemCount (); num++)
         {
             PluginItem *item = (PluginItem*)project->plugin->topLevelItem(num);

             if(item->getMode() != item->ModeDisable && item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg,0))
             {
                 item->plugindecoderinterface->decodeMsg(msg,0);
                 break;
             }
         }

         QColor color = qfile->checkMarker(msg);
         if(color.isValid())
         {
            return QVariant(QBrush(color));
         }
         else
         {
             if(project->settings->autoMarkFatalError && ( msg.getSubtypeString() == "error" || msg.getSubtypeString() == "fatal") ){
                return QVariant(QBrush(QColor(255,0,0)));
             }
             if(project->settings->autoMarkWarn && msg.getSubtypeString() == "warn"){
                return QVariant(QBrush(QColor(255,255,0)));
             }

             return QVariant(QBrush(QColor(255,255,255)));
         }
     }

     if ( role == Qt::TextAlignmentRole ) {
        switch(index.column())
        {
            case cnIndex:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case cnTime:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case cnTimeStamp:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case cnCounter:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case cnEcuId:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case cnAppId:
                switch(project->settings->showApIdDesc){
                case 0:
                    return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
                    break;
                case 1:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                default:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                }
            case cnContextId:
                switch(project->settings->showCtIdDesc){
                case 0:
                    return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
                    break;
                case 1:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                default:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                }
            case cnType:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case cnSubtype:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case cnMode:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case cnArgCount:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case cnPayload:
                return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
        }
    }

     return QVariant();
 }

  QVariant TableModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
 {
      if (role != Qt::DisplayRole)
          return QVariant();

      if (orientation == Qt::Horizontal)
      {
          switch(section)
          {
          case cnIndex:
              return QString("Index");
          case cnTime:
              return QString("Time");
          case cnTimeStamp:
              return QString("Timestamp");
          case cnCounter:
              return QString("Count");
          case cnEcuId:
              return QString("Ecuid");
          case cnAppId:
              switch(project->settings->showApIdDesc){
              case 0:
                   return QString("Apid");
              case 1:
                   return QString("Apid Desc");
              }
          case cnContextId:
              switch(project->settings->showCtIdDesc){
              case 0:
                   return QString("Ctid");
              case 1:
                   return QString("Ctid Desc");
              }
          case cnType:
              return QString("Type");
          case cnSubtype:
              return QString("Subtype");
          case cnMode:
              return QString("Mode");
          case cnArgCount:
              return QString("#Args");
          case cnPayload:
              return QString("Payload");
          }
      }

      return QVariant();
  }

 int TableModel::rowCount(const QModelIndex & /*parent*/) const
 {
    return qfile->sizeFilter();
 }

 void TableModel::modelChanged()
 {
     QModelIndex lIndex = index(0, 1);
     QModelIndex lLeft = index(qfile->sizeFilter()-1, 0);
     QModelIndex lRight = index(qfile->sizeFilter()-1, columnCount() - 1);
     emit(layoutChanged());
 }
