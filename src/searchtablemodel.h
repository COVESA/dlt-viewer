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
 * \file CSearchDialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef SEARCHTABLEMODEL_H
#define SEARCHTABLEMODEL_H

#include <QAbstractTableModel>

#include <cstdint>
#include <cstddef>
#include <vector>

#include "project.h"
#include "qdltpluginmanager.h"
#include "decodecacheservice.h"

#define DLT_VIEWER_SEARCHCOLUMN_COUNT FieldNames::Arg0

class CSearchTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    //! Construct the search result model.
    explicit CSearchTableModel(const QString &data, QObject *parent = 0);
    //! Destroy the search result model.
    ~CSearchTableModel();

    //! Return column header text and metadata.
    QVariant headerData(int section, Qt::Orientation orientation,
         int role = Qt::DisplayRole) const;

    //! Return model data for a search result row.
    QVariant data(const QModelIndex &index, int role) const;

    //! Return the number of search result rows.
    int rowCount(const QModelIndex & /*parent*/) const;
    //! Return the number of display columns.
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //! Notify views that model contents changed.
    void modelChanged();

    //! Remove all search results.
    void clear_SearchResults();
    //! Append a single search hit.
    void add_SearchResultEntry(unsigned long entry);
    //! Append multiple search hits.
    void add_SearchResultEntries(const std::vector<std::uint64_t> &entries);


    //! Return the current number of search hits.
    int get_SearchResultListSize() const;
    //! Read one search hit by position.
    bool get_SearchResultEntry(int position, unsigned long &entry);

    //! Determine background color for a decoded message.
    QColor getMsgBackgroundColor(QDltMsg &msg) const;

    /* pointer to the current loaded file */
    QDltFile *qfile;
    Project *project;
    QDltPluginManager *pluginManager;
    
signals:
    
public slots:


public:
    std::vector<unsigned long> m_searchResultList;
    mutable CDecodeCacheService m_decodeCacheService;
    
};

#endif // SEARCHTABLEMODEL_H
