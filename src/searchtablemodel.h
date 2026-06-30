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

#ifndef SEARCHTABLEMODEL_H
#define SEARCHTABLEMODEL_H

#include <QAbstractTableModel>

#include "project.h"
#include "qdltpluginmanager.h"
#include <qdltlrucache.hpp>

#include <optional>

#define DLT_VIEWER_SEARCHCOLUMN_COUNT FieldNames::Arg0

class SearchTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SearchTableModel(const QString &data, QObject *parent = 0);
    ~SearchTableModel();

    QVariant headerData(int section, Qt::Orientation orientation,
         int role = Qt::DisplayRole) const;

    QVariant data(const QModelIndex &index, int role) const;

    int rowCount(const QModelIndex & /*parent*/) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void modelChanged();

    void clear_SearchResults();
    void add_SearchResultEntry(unsigned long entry);
    void add_SearchResultEntries(const QList<unsigned long>& entries);


    int get_SearchResultListSize() const;
    bool get_SearchResultEntry(int position, unsigned long &entry);

    QColor getMsgBackgroundColor(QDltMsg &msg) const;

    /* pointer to the current loaded file */
    QDltFile *qfile;
    Project *project;
    QDltPluginManager *pluginManager;
    
signals:
    
public slots:


public:
    QList <unsigned long> m_searchResultList;

private:
    mutable QDltLruCache<unsigned long, std::optional<QDltMsg>> m_decodeCache{1024};
    mutable QDltLruCache<quint64, QVariant> m_renderCache{4096};

    /**
     * @brief Builds a render-cache key from message index and column.
     * @param msgIndex Message index.
     * @param column Column number.
     * @return Composite cache key.
     */
    quint64 renderCacheKey(unsigned long msgIndex, int column) const;

    /**
     * @brief Fetches and decodes a message with cache support.
     * @param msgIndex Message index.
     * @param msg Output decoded message.
     * @return True when message is available and decoded.
     */
    bool tryGetDecodedMsg(unsigned long msgIndex, QDltMsg &msg) const;

    /**
     * @brief Produces table display text/value for one cell.
     * @param index Target model index.
     * @param msg Decoded message buffer.
     * @param hasMessage True when @p msg contains valid data.
     * @param msgIndex Message index.
     * @return Cell display value.
     */
    QVariant buildDisplayData(const QModelIndex &index, QDltMsg &msg, bool hasMessage, unsigned long msgIndex) const;
    
};

#endif // SEARCHTABLEMODEL_H
