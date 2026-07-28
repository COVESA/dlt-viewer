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
 * \file searchsnapshot.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "searchsnapshot.h"

#include "qdltfile.h"

#include <QMutexLocker>

int SearchSnapshot::size() const
{
    return m_rows.size();
}

bool SearchSnapshot::isEmpty() const
{
    return m_rows.isEmpty();
}

quint64 SearchSnapshot::generationId() const
{
    return m_generation;
}

const QString &SearchSnapshot::fileName(int fileIndex) const
{
    return m_fileNames.at(fileIndex);
}

const SearchSnapshotRow &SearchSnapshot::rowAt(int rowIndex) const
{
    return m_rows.at(rowIndex);
}

std::shared_ptr<const SearchSnapshot> SearchSnapshotManager::capture(const QDltFile *file)
{
    if(file == nullptr)
        return std::shared_ptr<const SearchSnapshot>();

    QMutexLocker locker(&m_mutex);
    if(m_cachedFile == file && m_cachedSnapshot && file->isSearchSnapshotCurrent(m_cachedSnapshot->generationId()))
        return m_cachedSnapshot;

    auto snapshot = std::make_shared<SearchSnapshot>(file->captureSearchSnapshot());
    m_cachedFile = file;
    m_cachedSnapshot = snapshot;
    return snapshot;
}

void SearchSnapshotManager::invalidate(const QDltFile *file)
{
    QMutexLocker locker(&m_mutex);
    if(file != nullptr && m_cachedFile != file)
        return;

    m_cachedFile = nullptr;
    m_cachedSnapshot.reset();
}