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
 * \file decodecacheservice.cpp
 * @licence end@
 */

#include "decodecacheservice.h"

#include "messagestore.h"
#include "qdltfile.h"
#include "qdltpluginmanager.h"

#include <utility>

CDecodeCacheService::CDecodeCacheService()
{
    m_cache.reserve(static_cast<std::size_t>(kMaxEntries));
}

bool CDecodeCacheService::message(const QDltFile *file,
                                 QDltPluginManager *pluginManager,
                                 int globalIndex,
                                 bool decodeEnabled,
                                 int triggeredByUser,
                                 QDltMsg &msg,
                                 bool useCache,
                                 bool singlePassBypass)
{
    if (!file || globalIndex < 0 || globalIndex >= file->size())
        return false;

    CQDltFileMessageStoreAdapter messageStore(file);

    // Single-pass scatter bypass skips cache I/O when each message is accessed only once.
    if (singlePassBypass)
    {
        QDltMsg loaded;
        if (!messageStore.message(static_cast<MessageId>(globalIndex), loaded))
            return false;
        if (decodeEnabled && !decode(pluginManager, triggeredByUser, loaded))
            return false;
        msg = loaded;
        return true;
    }

    const std::uint64_t decodePipelineGeneration =
        (decodeEnabled && pluginManager) ? pluginManager->decodePipelineGeneration() : 0;
    const CacheKey key{file,
                       globalIndex,
                       decodeEnabled,
                       triggeredByUser,
                       decodePipelineGeneration};

    if (useCache)
    {
        std::lock_guard<std::mutex> locker(m_cacheLock);
        const auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            const bool pipelineUnchanged = !decodeEnabled || !pluginManager ||
                pluginManager->decodePipelineGeneration() == decodePipelineGeneration;
            if (pipelineUnchanged)
            {
                msg = it->second;
                return true;
            }
        }
    }

    QDltMsg loaded;
    if (!messageStore.message(static_cast<MessageId>(globalIndex), loaded))
        return false;

    if (decodeEnabled)
        decode(pluginManager, triggeredByUser, loaded);

    const bool pipelineUnchanged = !decodeEnabled || !pluginManager ||
        pluginManager->decodePipelineGeneration() == decodePipelineGeneration;

    if (useCache && pipelineUnchanged)
    {
        std::lock_guard<std::mutex> locker(m_cacheLock);
        const auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            msg = it->second;
            return true;
        }

        m_cache.emplace(key, loaded);
        m_fifoOrder.push_back(key);
        // pruneIfNeeded() relies on this lock being held for the entire eviction pass.
        pruneIfNeeded();
    }

    msg = loaded;
    return true;
}

bool CDecodeCacheService::decode(QDltPluginManager *pluginManager,
                                 int triggeredByUser,
                                 QDltMsg &msg) const
{
    if (!pluginManager)
        return true;

    const std::uint64_t decodePipelineGeneration = pluginManager->decodePipelineGeneration();
    pluginManager->decodeMsg(msg, triggeredByUser);
    return pluginManager->decodePipelineGeneration() == decodePipelineGeneration;
}

void CDecodeCacheService::clear()
{
    std::lock_guard<std::mutex> locker(m_cacheLock);
    m_cache.clear();
    m_fifoOrder.clear();
}

void CDecodeCacheService::clearForFile(const QDltFile *file)
{
    if (!file)
        return;

    std::lock_guard<std::mutex> locker(m_cacheLock);
    for (auto it = m_cache.begin(); it != m_cache.end(); )
    {
        if (it->first.file == file)
            it = m_cache.erase(it);
        else
            ++it;
    }

    std::deque<CacheKey> filtered;
    for (const CacheKey &key : m_fifoOrder)
    {
        if (key.file != file)
            filtered.push_back(key);
    }
    m_fifoOrder = std::move(filtered);
}

void CDecodeCacheService::pruneIfNeeded()
{
    if (m_cache.size() <= static_cast<std::size_t>(kMaxEntries))
        return;

    const std::size_t pruneCount = m_cache.size() - static_cast<std::size_t>(kMaxEntries);
    std::size_t removed = 0;

    while (removed < pruneCount && !m_fifoOrder.empty())
    {
        const CacheKey key = m_fifoOrder.front();
        m_fifoOrder.pop_front();
        if (m_cache.erase(key) != 0)
            ++removed;
    }
}
