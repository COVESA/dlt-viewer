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
 * \file treemodel.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QMutex>

#include "project.h"
#include "qdlt.h"

#define DLT_VIEWER_LIST_BUFFER_SIZE 100024
#define DLT_VIEWER_COLUMN_COUNT 12

extern "C"
{
        #include "dlt_common.h"
        #include "dlt_user_shared.h"
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
    void modelChanged();
private:
    enum ColumnNames
    {
        cnIndex,
        cnTime,
        cnTimeStamp,
        cnCounter,
        cnEcuId,
        cnAppId,
        cnContextId,
        cnType,
        cnSubtype,
        cnMode,
        cnArgCount,
        cnPayload
    };
};

#endif // TABLEMODEL_H
