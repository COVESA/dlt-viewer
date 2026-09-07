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
#include <QHeaderView>
#include <QEvent>
#include <QToolTip>
#include <QVector>

#include "project.h"
#include "qdltpluginmanager.h"
#include "fieldnames.h"
#include <qdltlrucache.hpp>

#include <optional>

#define DLT_VIEWER_COLUMN_COUNT FieldNames::Arg0

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
    QString getToolTipForFields(FieldNames::Fields cn);

private:
    struct DecodedMsgCacheEntry
    {
        long int filterPosIndex;
        std::optional<QDltMsg> msg;
    };

    long int lastSearchIndex;
    bool emptyForceFlag;
    bool loggingOnlyMode;

    // Dedup getMsg()/decodeMsg() across roles and columns for recently rendered rows.
    mutable QDltLruCache<int, DecodedMsgCacheEntry> m_cache{512};

    std::optional<QDltMsg> getDecodedMsg(int row, long int filterposindex) const;
    QVariant buildDisplayValue(int column, long int filterPosIndex, const std::optional<QDltMsg> &msg) const;

    long int searchhit;
    QColor searchBackgroundColor() const;
    QColor searchhit_higlightColor;
    QColor manualMarkerColor;
    QList<unsigned long int> selectedMarkerRows;
    QColor getMsgBackgroundColor(const std::optional<QDltMsg>& msg, int index, long int filterposindex) const;
    bool eventFilter(QObject *obj, QEvent *event);
};

class HtmlDelegate : public QStyledItemDelegate
{
protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // TABLEMODEL_H
