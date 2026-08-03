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
 * \file searchdialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */
#include "searchtablemodel.h"

#include "fieldnames.h"
#include "dltuiutils.h"
#include "dlt_protocol.h"
#include "qdltoptmanager.h"



SearchTableModel::SearchTableModel(const QString &,QObject *parent) :
    QAbstractTableModel(parent)
{
    qfile = NULL;
    project = NULL;
    pluginManager = NULL;
    m_renderCacheGeneration = 0;
}

SearchTableModel::~SearchTableModel()
{

}

QVariant SearchTableModel::buildDisplayValue(int column, unsigned long messageIndex, QDltMsg &msg) const
{
    QString visu_data;
    switch(column)
    {
    case FieldNames::Index:
        /* display index */
        return QString("%L1").arg(messageIndex);
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
        switch(project->settings->showApIdDesc){
        case 0:
            return msg.getApid();
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
         default:
            return msg.getApid();
        }
    case FieldNames::ContextId:
        switch(project->settings->showCtIdDesc){
        case 0:
            return msg.getCtid();
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
         default:
            return msg.getCtid();
        }
    case FieldNames::SessionId:
        switch(project->settings->showSessionName){
        case 0:
            return QString("%1").arg(msg.getSessionid());
        case 1:
            if(!msg.getSessionName().isEmpty())
               return msg.getSessionName();
           else
               return QString("%1").arg(msg.getSessionid());
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
        /* display payload */
        visu_data = msg.toStringPayload().simplified().remove(QChar::Null);
        if(qfile) qfile->applyRegExString(msg,visu_data);
        return visu_data;
    case FieldNames::MessageId:
        return QString::asprintf(project->settings->msgIdFormat.toUtf8(),msg.getMessageId());
    default:
        if (column>=FieldNames::Arg0)
        {
            int col=column-FieldNames::Arg0; //arguments a zero based
            QDltArgument arg;
            if (msg.getArgument(col,arg))
            {
                return arg.toString();
            }
            else
             return QString(" - ");
        }
    }

    return QVariant();
}

SearchTableModel::DecodeRenderCacheEntry SearchTableModel::buildDecodeRenderCacheEntry(unsigned long messageIndex, QDltMsg &msg) const
{
    DecodeRenderCacheEntry entry;
    entry.messageIndex = messageIndex;
    entry.generation = m_renderCacheGeneration;

    const int currentColumnCount = columnCount();
    entry.displayValues.reserve(currentColumnCount);
    for (int column = 0; column < currentColumnCount; ++column)
    {
        entry.displayValues.push_back(buildDisplayValue(column, messageIndex, msg));
    }

    return entry;
}

QVariant SearchTableModel::data(const QModelIndex &index, int role) const
{
    QDltMsg msg;

    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_searchResultList.size() || index.row()<0)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        /* get the message with the selected item id */
        unsigned long messageIndex = m_searchResultList.at(index.row());
        if(!qfile->getMsg(messageIndex, msg))
        {
            if(index.column() == FieldNames::Index)
            {
                return QString("%1").arg(messageIndex);
            }
            else if(index.column() == FieldNames::Payload)
            {
                return QString("!!CORRUPTED MESSAGE!!");
            }
            return QVariant();
        }

        if(QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool())
            pluginManager->decodeMsg(msg,!QDltOptManager::getInstance()->issilentMode());

        DecodeRenderCacheEntry entry;
        const bool hasRenderCacheEntry = m_decodeRenderCache.exists(index.row());
        if (hasRenderCacheEntry)
        {
            entry = m_decodeRenderCache.get(index.row());
        }

        const bool isRenderCacheValid = hasRenderCacheEntry
                                        && entry.messageIndex == messageIndex
                                        && entry.generation == m_renderCacheGeneration
                                        && entry.displayValues.size() == columnCount();

        if (!isRenderCacheValid)
        {
            entry = buildDecodeRenderCacheEntry(messageIndex, msg);
            m_decodeRenderCache.put(index.row(), entry);
        }

        if (index.column() < 0 || index.column() >= entry.displayValues.size())
        {
            return QVariant();
        }

        return entry.displayValues.at(index.column());
    }

    if ( role == Qt::ForegroundRole )
    {
        if(qfile->getMsg(m_searchResultList.at(index.row()), msg))
        {
            /* Valid message found, calculate background color and find optimal forground color */
            return QVariant(QBrush(DltUiUtils::optimalTextColor(getMsgBackgroundColor(msg))));
        }
        /* default return black forground color */
        QColor brushColor = QColor(0,0,0);

        if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
        {
            brushColor = QColor(255,255,255);
        }

        return QVariant(QBrush(brushColor));
    }

    if ( role == Qt::BackgroundRole )
    {
        if(qfile->getMsg(m_searchResultList.at(index.row()), msg))
        {
            /* Valid message found, calculate background color */
            return QVariant(QBrush(getMsgBackgroundColor(msg)));
        }
        /* default return white background color */
        QColor brushColor = QColor(255,255,255);

        if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
        {
            brushColor = QColor(31,31,31);
        }

        return QVariant(QBrush(brushColor));
    }


    if ( role == Qt::TextAlignmentRole ) {
       /*switch(index.column())
       {
           //override default alignment here if needed
           //case FieldNames::Index: return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
           default:

       }*/
       return FieldNames::getColumnAlignment((FieldNames::Fields)index.column(),project->settings);
   }

    return QVariant();
}

QVariant SearchTableModel::headerData(int section, Qt::Orientation orientation,
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
           /*switch(section)
               {
                 //override default alignment here if needed
                 //case FieldNames::Payload: return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
                 default:
               }*/
           return FieldNames::getColumnAlignment((FieldNames::Fields)section,project->settings);
           }
        default:
           break;
       }
   }

   return QVariant();
}

int SearchTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return get_SearchResultListSize();
}

void SearchTableModel::modelChanged()
{
    if (!m_searchResultList.isEmpty())
    {
        index(0, 1);
        index(m_searchResultList.size()-1, 0);
        index(m_searchResultList.size()-1, columnCount() - 1);
    }
    ++m_renderCacheGeneration;
    emit(layoutChanged());
}

int SearchTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return DLT_VIEWER_SEARCHCOLUMN_COUNT+project->settings->showArguments;
}

void SearchTableModel::clear_SearchResults()
{
    beginResetModel();
    m_searchResultList.clear();
    ++m_renderCacheGeneration;
    endResetModel();
}

void SearchTableModel::add_SearchResultEntry(unsigned long entry)
{
    const int row = m_searchResultList.size();
    beginInsertRows(QModelIndex(), row, row);
    m_searchResultList.append(entry);
    endInsertRows();
}

void SearchTableModel::add_SearchResultEntries(const QList<unsigned long>& entries)
{
    if (entries.isEmpty())
        return;

    const int firstRow = m_searchResultList.size();
    const int lastRow = firstRow + entries.size() - 1;

    beginInsertRows(QModelIndex(), firstRow, lastRow);
    m_searchResultList.append(entries);
    endInsertRows();
}


bool SearchTableModel::get_SearchResultEntry(int position, unsigned long &entry)
{
    if (position > m_searchResultList.size() || 0 > position )
    {
        return false;
    }

    entry = m_searchResultList.at(position);
    return true;
}


int SearchTableModel::get_SearchResultListSize() const
{
    return m_searchResultList.size();
}

QColor SearchTableModel::getMsgBackgroundColor(QDltMsg &msg) const
{
    /* get check marker color */
    QColor color = qfile->checkMarker(msg);
    if(color.isValid())
    {
        /* Valid marker found, use background color as defined in marker */
        return color;
    }
    else
    {
        if(project->settings->autoMarkFatalError && ( msg.getSubtypeString() == "error" || msg.getSubtypeString() == "fatal") )
        {
           /* If automark error is enabled, set red as background color */
           return QColor(255,0,0);
        }
        if(project->settings->autoMarkWarn && msg.getSubtypeString() == "warn")
        {
            /* If automark warning is enabled, set red as background color */
           return QColor(255,255,0);
        }
        if(project->settings->autoMarkMarker && msg.getType()==QDltMsg::DltTypeControl &&
           msg.getSubtype()==QDltMsg::DltControlResponse && msg.getCtrlServiceId() == DLT_SERVICE_ID_MARKER)
        {
            /* If automark marker is enabled, set green as background color */
           return QColor(0,255,0);
        }
    }

    /* default return white background color */
    QColor brushColor = QColor(255,255,255);

    if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
    {
        brushColor = QColor(31,31,31);
    }

    return brushColor;
}
