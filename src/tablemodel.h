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
#include "decodecacheservice.h"
#include <qdltlrucache.hpp>

#include <optional>
#include <vector>

#define DLT_VIEWER_COLUMN_COUNT FieldNames::Arg0

class CTableModel : public QAbstractTableModel
{
Q_OBJECT

public:
    //! Construct the main table model.
    CTableModel(const QString &data, QObject *parent = 0);
    //! Destroy the main table model.
    ~CTableModel();

    //! Return data for a table cell.
    QVariant data(const QModelIndex &index, int role) const;
    //! Return header data for a column or row.
    QVariant headerData(int section, Qt::Orientation orientation,
         int role = Qt::DisplayRole) const;
    //! Return current row count for the active view.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    //! Return number of visible columns.
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /* pointer to the current loaded file */
    QDltFile *qfile;
    Project *project;
    QDltPluginManager *pluginManager;
    //! Notify attached views that the model content changed.
    void modelChanged();
    //! Set marker highlight at a specific row.
    int setMarker(long int lineindex, QColor hlcolor); //used in search functionality
    //! Set manual marker highlight for selected rows.
    int setManualMarker(QList<unsigned long int> selectedMarkerRows, QColor hlcolor); //used in mainwindow
    void setForceEmpty(bool emptyForceFlag) { this->emptyForceFlag = emptyForceFlag; }
    void setLoggingOnlyMode(bool loggingOnlyMode) { this->loggingOnlyMode = loggingOnlyMode; }
    void setLastSearchIndex(int idx) {this->lastSearchIndex = idx;}
    //! Return tooltip text for a given field/column.
    QString getToolTipForFields(FieldNames::Fields cn);

private:
    long int lastSearchIndex;
    bool emptyForceFlag;
    bool loggingOnlyMode;

    // cache is used in data()-method to avoid decoding of the same message multiple times
    // key is a message index in the qdltfile; message can fail to decode, in that case value is empty optional
    mutable QDltLruCache<int, std::optional<QDltMsg>> m_cache{1};
    mutable std::vector<int> m_filteredProjectionCache;
    mutable CDecodeCacheService m_decodeCacheService;

    long int searchhit;
    QColor searchBackgroundColor() const;
    QColor searchhit_higlightColor;
    QColor manualMarkerColor;
    QList<unsigned long int> selectedMarkerRows;
    //! Resolve model row index to global message index.
    int resolveGlobalIndexForRow(int row) const;
    //! Compute message background color for a row.
    QColor getMsgBackgroundColor(const std::optional<QDltMsg>& msg, int index, long int filterposindex) const;
    //! Handle tooltip and related item-view events.
    bool eventFilter(QObject *obj, QEvent *event);
};

class CHtmlDelegate : public QStyledItemDelegate
{
protected:
    //! Paint rich-text content in item delegates.
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    //! Return rich-text item size hint.
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // TABLEMODEL_H
