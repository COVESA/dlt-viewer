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
 * \file treemodel.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QMutex>
#include <QStyledItemDelegate>

#include "project.h"
#include "qdltpluginmanager.h"

#define DLT_VIEWER_LIST_BUFFER_SIZE 100024
#define DLT_VIEWER_COLUMN_COUNT FieldNames::Arg0

extern "C"
{
}

class TableModel : public QAbstractTableModel
{
Q_OBJECT

public:
    TableModel(const QString &data, QObject *parent = 0);
    ~TableModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
         int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /* pointer to the current loaded file */
    QDltFile *qfile;
    Project *project;
    QDltPluginManager *pluginManager;
    void modelChanged();
    int setMarker(long int lineindex, QColor hlcolor); //used in search functionality
    int setManualMarker(QList<unsigned long int> selectedMarkerRows, QColor hlcolor); //used in mainwindow
    void setForceEmpty(bool emptyForceFlag) { this->emptyForceFlag = emptyForceFlag; }
    void setLoggingOnlyMode(bool loggingOnlyMode) { this->loggingOnlyMode = loggingOnlyMode; }
    void setLastSearchIndex(int idx) {this->lastSearchIndex = idx;}

private:
    long int lastSearchIndex;
    bool emptyForceFlag;
    bool loggingOnlyMode;

    long int searchhit;
    QColor searchBackgroundColor() const;
    QColor searchhit_higlightColor;
    QColor manualMarkerColor;
    QList<unsigned long int> selectedMarkerRows;
    QColor getMsgBackgroundColor(QDltMsg &msg,int index,long int filterposindex) const;
};

class HtmlDelegate : public QStyledItemDelegate
{
protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // TABLEMODEL_H
