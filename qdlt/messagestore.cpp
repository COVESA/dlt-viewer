/**
 * @licence app begin@
 * Copyright (C) 2026
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file messagestore.cpp
 * @licence end@
 */

#include "messagestore.h"

#include "qdltfile.h"

CQDltFileMessageStoreAdapter::CQDltFileMessageStoreAdapter(const QDltFile *file)
    : m_file(file)
{
}

void CQDltFileMessageStoreAdapter::setFile(const QDltFile *file)
{
    m_file = file;
}

const QDltFile *CQDltFileMessageStoreAdapter::file() const
{
    return m_file;
}

std::size_t CQDltFileMessageStoreAdapter::size() const
{
    return m_file ? static_cast<std::size_t>(m_file->size()) : 0;
}

bool CQDltFileMessageStoreAdapter::contains(MessageId messageId) const
{
    return m_file && messageId < static_cast<MessageId>(m_file->size());
}

MessageId CQDltFileMessageStoreAdapter::messageIdForGlobalIndex(int index) const
{
    if (!m_file || index < 0 || index >= m_file->size())
        return kInvalidMessageId;

    return static_cast<MessageId>(index);
}

MessageId CQDltFileMessageStoreAdapter::messageIdForFilteredRow(int row) const
{
    if (!m_file)
        return kInvalidMessageId;

    const int msgIndex = m_file->filteredGlobalIndexAt(row);
    if (msgIndex < 0)
        return kInvalidMessageId;

    return static_cast<MessageId>(msgIndex);
}

int CQDltFileMessageStoreAdapter::globalIndexForMessageId(MessageId messageId) const
{
    return contains(messageId) ? static_cast<int>(messageId) : -1;
}

std::vector<char> CQDltFileMessageStoreAdapter::rawMessage(MessageId messageId) const
{
    if (!contains(messageId))
        return {};

    const QByteArray data = m_file->messageBytesAt(static_cast<int>(messageId));
    return std::vector<char>(data.cbegin(), data.cend());
}

bool CQDltFileMessageStoreAdapter::message(MessageId messageId, QDltMsg &msg, bool useCache) const
{
    if (!contains(messageId))
        return false;

    const int globalIndex = static_cast<int>(messageId);
    if (!m_file->messageAt(globalIndex, msg, useCache))
        return false;

    return true;
}

std::vector<MessageId> CQDltFileMessageStoreAdapter::snapshotAllMessageIds() const
{
    std::vector<MessageId> ids;
    if (!m_file)
        return ids;

    const int total = m_file->size();
    ids.reserve(total);
    for (int i = 0; i < total; ++i)
        ids.push_back(static_cast<MessageId>(i));

    return ids;
}

std::vector<MessageId> CQDltFileMessageStoreAdapter::snapshotFilteredMessageIds() const
{
    if (!m_file)
        return {};

    if (!m_file->isFilter())
        return snapshotAllMessageIds();

    std::vector<MessageId> ids;
    const auto &filterRef = m_file->getIndexFilterRef();
    ids.reserve(filterRef.size());
    for (const auto &index : filterRef)
    {
        if (index >= 0)
            ids.push_back(static_cast<MessageId>(index));
    }

    return ids;
}
