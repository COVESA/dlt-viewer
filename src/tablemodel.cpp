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
#include <QApplication>
#include <qmessagebox.h>

#include "tablemodel.h"
#include "fieldnames.h"
#include "dltuiutils.h"
#include "dlt_protocol.h"
#include "regex_search_replace.h"

static long int lastrow = -1; // necessary because object tablemodel can not be changed, so no member variable can be used
char buffer[DLT_VIEWER_LIST_BUFFER_SIZE];


void getmessage( int indexrow, long int filterposindex, unsigned int* decodeflag, QDltMsg* msg, QDltMsg* lastmsg, QDltFile* qfile, bool* success )
{
 if ( indexrow == lastrow)
 {
  *msg = *lastmsg;
 }
 else
 {
  *success = qfile->getMsg(filterposindex, *msg);
  *lastmsg = *msg;
  *decodeflag = 1;
 }
 if ( indexrow == 0)
 {
  lastrow = 0;
 }
 else
 {
  lastrow = indexrow;
 }
}


TableModel::TableModel(const QString & /*data*/, QObject *parent)
     : QAbstractTableModel(parent)
 {
     qfile = NULL;
     project = NULL;
     pluginManager = NULL;
     lastSearchIndex = -1;
     emptyForceFlag = false;
     loggingOnlyMode = false;
     searchhit = -1;
     lastrow = -1;
 }

 TableModel::~TableModel()
 {

 }


 int TableModel::columnCount(const QModelIndex & /*parent*/) const
 {
     return DLT_VIEWER_COLUMN_COUNT+project->settings->showArguments;
 }


 QVariant TableModel::data(const QModelIndex &index, int role) const
 {
     QByteArray buf;
     static QDltMsg msg;
     static QDltMsg lastmsg;
     static QDltMsg last_decoded_msg;
     static unsigned int decodeflag = 0;
     static bool success = true;

     long int filterposindex = 0;


     if (index.isValid() == false)
     {
         return QVariant();
     }

     if (index.row() >= qfile->sizeFilter() && index.row()<0)
     {
         return QVariant();
     }

     filterposindex = qfile->getMsgFilterPos(index.row());

     if (role == Qt::DisplayRole)
     {
         /* get the message with the selected item id */
         if(true == loggingOnlyMode)
         {
             msg = QDltMsg();
         }
         else
         {
           getmessage( index.row(), filterposindex, &decodeflag, &msg, &lastmsg, qfile, &success);

           if ( success == false )
           {
             if(index.column() == FieldNames::Index)
             {
                 return QString("%1").arg(qfile->getMsgFilterPos(index.row()));
             }
             else if(index.column() == FieldNames::Payload)
             {
                 qDebug() << "Corrupted message at index" << index.row();
                 return QString("!!CORRUPTED MESSAGE!!");
             }
             return QVariant();
          }
         }

         if((QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool()))
         {
             if ( decodeflag == 1 )
              {
               decodeflag = 0;
               last_decoded_msg = msg;
               pluginManager->decodeMsg(msg,!QDltOptManager::getInstance()->issilentMode());
               last_decoded_msg = msg;
              }
              else
              {
                msg = last_decoded_msg;
              }
         }

         QString visu_data;
         switch(index.column())
         {
         case FieldNames::Index:
             /* display index */
             return QString("%L1").arg(qfile->getMsgFilterPos(index.row()));
         case FieldNames::Time:
             if( project->settings->automaticTimeSettings == 0 )
                return QString("%1.%2").arg(msg.getGmTimeWithOffsetString(project->settings->utcOffset,project->settings->dst)).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
             else
                return QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
         case FieldNames::TimeStamp:
             return QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
         case FieldNames::Counter:
             return QString("%1").arg(msg.getMessageCounter());
         case FieldNames::EcuId:
             return msg.getEcuid();
         case FieldNames::AppId:
             switch(project->settings->showApIdDesc)
             {
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
         case FieldNames::ContextId:
             switch(project->settings->showCtIdDesc)
             {
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
         case FieldNames::SessionId:
             switch(project->settings->showSessionName){
             case 0:
                 return QString("%1").arg(msg.getSessionid());
                 break;
             case 1:
                 if(!msg.getSessionName().isEmpty())
                 {
                    return msg.getSessionName();
                 }
                else
                 {
                    return QString("%1").arg(msg.getSessionid());
                 }
                 break;
              default:
                 return QString("%1").arg(msg.getSessionid());
             }
         case FieldNames::Type:
             return msg.getTypeString();
         case FieldNames::Subtype:
             return msg.getSubtypeString();
         case FieldNames::Mode:
             return msg.getModeString();
         case FieldNames::ArgCount:
             return QString("%1").arg(msg.getNumberOfArguments());
         case FieldNames::Payload:
             if( true == loggingOnlyMode)
             {
                 return QString("Logging only Mode! Disable in Project Settings!");
             }
             /* display payload */
             visu_data = msg.toStringPayload().trimmed().replace('\n', ' ');

             if((QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool()))
             {
                 for(int num = 0; num < project->filter->topLevelItemCount (); num++) {
                     FilterItem *item = (FilterItem*)project->filter->topLevelItem(num);
                     if(item->checkState(0) == Qt::Checked && item->filter.enableRegexSearchReplace) {
                         apply_regex_string(visu_data, item->filter.regex_search, item->filter.regex_replace);
                     }
                 }
             }

             return visu_data;
         case FieldNames::MessageId:
             return QString().sprintf(project->settings->msgIdFormat.toLatin1() ,msg.getMessageId());
         default:
             if (index.column()>=FieldNames::Arg0)
             {
                 int col=index.column()-FieldNames::Arg0; //arguments a zero based
                 QDltArgument arg;
                 if (msg.getArgument(col,arg))
                 {
                     return arg.toString();
                 }
                 else
                 {
                  return QString(" - ");
                 }

             }
         }
     }

     if ( role == Qt::ForegroundRole )
     {
         getmessage( index.row(), filterposindex, &decodeflag, &msg, &lastmsg, qfile, &success); // version2

         // Color the last search row
         if(lastSearchIndex != -1 && filterposindex == qfile->getMsgFilterPos(lastSearchIndex))
         {
             return QVariant(QBrush(DltUiUtils::optimalTextColor(searchBackgroundColor())));
         }
         else if (QColor(qfile->checkMarker(msg)).isValid())
         {
           QColor color = qfile->checkMarker(msg);
           return QVariant(QBrush(DltUiUtils::optimalTextColor(color)));
         }
         else if(project->settings->autoMarkFatalError && !QColor(qfile->checkMarker(msg)).isValid() && ( msg.getSubtypeString() == "error" || msg.getSubtypeString() == "fatal")  )
         {
            return QVariant(QBrush(QColor(255,255,255)));
         }
         else
         {
            return QVariant(QBrush(QColor(0,0,0)));
         }
     }

     if ( role == Qt::BackgroundRole )
     {
         getmessage( index.row(), filterposindex, &decodeflag, &msg, &lastmsg, qfile, &success); // version2

         if((QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool()))
         {
             if ( decodeflag == 1 )
              {
               decodeflag = 0;
               last_decoded_msg = msg;
               pluginManager->decodeMsg(msg,!QDltOptManager::getInstance()->issilentMode());
               last_decoded_msg = msg;
              }
              else
              {
                msg = last_decoded_msg;
              }
         }

         QColor color = qfile->checkMarker(msg);

         if(color.isValid())
         {
            return QVariant(QBrush(color));
         }
         else
         {
             if ( searchhit > -1 && searchhit == index.row() )
             {
               return QVariant(QBrush(searchhit_higlightColor));
             }
             if ( selectedMarkerRows.contains(index.row()) )
             {
               return QVariant(QBrush(manualMarkerColor));
             }
             if(project->settings->autoMarkFatalError && ( msg.getSubtypeString() == "error" || msg.getSubtypeString() == "fatal") )
             {
                return QVariant(QBrush(QColor(255,0,0)));
             }
             if(project->settings->autoMarkWarn && msg.getSubtypeString() == "warn")
             {
                return QVariant(QBrush(QColor(255,255,0)));
             }
             if(project->settings->autoMarkMarker && msg.getType()==QDltMsg::DltTypeControl &&
                msg.getSubtype()==QDltMsg::DltControlResponse && msg.getCtrlServiceId() == DLT_SERVICE_ID_MARKER)
             {
                return QVariant(QBrush(QColor(0,255,0)));
             }
             return QVariant(QBrush(QColor(255,255,255))); // this is the default background clor
         }
     }

     if ( role == Qt::TextAlignmentRole )
     {
         switch(index.column())
        {
            //case FieldNames::Index: return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            default:
                  return FieldNames::getColumnAlignment((FieldNames::Fields)index.column(),project->settings);

        }
    }

     return QVariant();
 }

QVariant TableModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{    
    if (orientation == Qt::Horizontal)
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return FieldNames::getName((FieldNames::Fields)section, project->settings);
        case Qt::TextAlignmentRole:
            {
            switch(section)
                {
                 //case FieldNames::Payload: return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
                  default:return FieldNames::getColumnAlignment((FieldNames::Fields)section,project->settings);
                }

            }
         default:
            break;
        }
    }

    return QVariant();
}

 int TableModel::rowCount(const QModelIndex & /*parent*/) const
 {
     if(true == emptyForceFlag)
         return 0;
     else if(true == loggingOnlyMode)
         return 1;
     else
         return qfile->sizeFilter();
 }

 void TableModel::modelChanged()
 {
     if(true == emptyForceFlag)
     {
         index(0, 1);
         index(qfile->sizeFilter()-1, 0);
         index(qfile->sizeFilter()-1, columnCount() - 1);
     }
     else
     {
         index(0, 1);
         index(0, 0);
         index(0, columnCount() - 1);
     }
     lastrow = -1;
     emit(layoutChanged());
 }

int TableModel::setManualMarker(QList<unsigned long int> selectedRows, QColor hlcolor) //used in mainwindow
{
manualMarkerColor = hlcolor;
this->selectedMarkerRows = selectedRows;
return 0;
}

int TableModel::setMarker(long int lineindex, QColor hlcolor)
{
  searchhit_higlightColor = hlcolor;
  searchhit = lineindex;
  return 0;
}

QColor TableModel::searchBackgroundColor() const
{
    QString color = QDltSettingsManager::getInstance()->value("other/searchResultColor", QString("#00AAFF")).toString();
    QColor hlColor(color);
    return hlColor;
}

void HtmlDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionV4 = option;
    initStyleOption(&optionV4, index);

    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(optionV4.text);

    /// Painting item without text
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (optionV4.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText));

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();

}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionV4 = option;
    initStyleOption(&optionV4, index);

    QTextDocument doc;
    doc.setHtml(optionV4.text);
    doc.setTextWidth(optionV4.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}
