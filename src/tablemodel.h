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

    /**
     * @brief Announces appended rows for incremental live updates.
     * @param firstRow First appended row.
     * @param lastRow Last appended row.
     */
    void appendRows(int firstRow, int lastRow);
    int setMarker(long int lineindex, QColor hlcolor); //used in search functionality
    int setManualMarker(QList<unsigned long int> selectedMarkerRows, QColor hlcolor); //used in mainwindow
    void setForceEmpty(bool emptyForceFlag) { this->emptyForceFlag = emptyForceFlag; }
    void setLoggingOnlyMode(bool loggingOnlyMode) { this->loggingOnlyMode = loggingOnlyMode; }
    void setLastSearchIndex(int idx) {this->lastSearchIndex = idx;}
    QString getToolTipForFields(FieldNames::Fields cn);

private:
    long int lastSearchIndex;
    bool emptyForceFlag;
    bool loggingOnlyMode;

    // cache is used in data()-method to avoid decoding of the same message multiple times
    // key is a message index in the qdltfile; message can fail to decode, in that case value is empty optional
    mutable QDltLruCache<long int, std::optional<QDltMsg>> m_cache{1};
    // Cache formatted display values for recently painted cells to reduce UI-thread formatting churn.
    mutable QDltLruCache<quint64, QVariant> m_renderCache{4096};

    long int searchhit;

    /**
     * @brief Builds a cache key for rendered cell values.
     * @param row Row index.
     * @param column Column index.
     * @return Composite cache key.
     */
    quint64 renderCacheKey(int row, int column) const;

    /**
     * @brief Formats display value for a table cell.
     * @param index Target model index.
     * @param msg Optional decoded message.
     * @param filterposindex Message index in filtered view.
     * @return Rendered cell value.
     */
    QVariant buildDisplayData(const QModelIndex &index, std::optional<QDltMsg> &msg, long int filterposindex) const;
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
