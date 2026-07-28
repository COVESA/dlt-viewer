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
 * \file searchsnapshot.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef SEARCHSNAPSHOT_H
#define SEARCHSNAPSHOT_H

#include "export_rules.h"

#include <QMutex>
#include <QStringList>
#include <QVector>

#include <memory>

class QDltFile;

class QDLT_EXPORT SearchSnapshotRow
{
public:
    int messageIndex{-1};
    int fileIndex{-1};
    qint64 filePosition{-1};
    qint64 byteCount{0};
};

class QDLT_EXPORT SearchSnapshot
{
public:
    int size() const;
    bool isEmpty() const;
    quint64 generationId() const;
    const QString &fileName(int fileIndex) const;
    const SearchSnapshotRow &rowAt(int rowIndex) const;

private:
    friend class QDltFile;
    friend class SearchSnapshotManager;

    QVector<SearchSnapshotRow> m_rows;
    QStringList m_fileNames;
    quint64 m_generation{0};
};

class QDLT_EXPORT SearchSnapshotManager
{
public:
    std::shared_ptr<const SearchSnapshot> capture(const QDltFile *file);
    void invalidate(const QDltFile *file = nullptr);

private:
    mutable QMutex m_mutex;
    const QDltFile *m_cachedFile{nullptr};
    std::shared_ptr<const SearchSnapshot> m_cachedSnapshot;
};

#endif // SEARCHSNAPSHOT_H