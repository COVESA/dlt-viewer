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
 * \file decodecacheservice.h
 * @licence end@
 */

#ifndef DECODECACHESERVICE_H
#define DECODECACHESERVICE_H

#include "export_rules.h"
#include "qdltmsg.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <vector>

class QDltFile;
class QDltPluginManager;

class QDLT_EXPORT CDecodeCacheService
{
public:
    //! Create an empty decode cache service.
    CDecodeCacheService();

    //! Retrieve and decode a message, optionally using the cache.
    bool message(const QDltFile *file,
                 QDltPluginManager *pluginManager,
                 int globalIndex,
                 bool decodeEnabled,
                 int triggeredByUser,
                 QDltMsg &msg,
                 bool useCache = true);

    //! Decode an already-loaded message through plugin manager.
    bool decode(QDltPluginManager *pluginManager,
                int triggeredByUser,
                QDltMsg &msg) const;

    //! Clear all cached decode entries.
    void clear();
    //! Clear cached entries that belong to a specific file.
    void clearForFile(const QDltFile *file);

private:
    struct CacheKey
    {
        const QDltFile *file;
        int globalIndex;
        bool decodeEnabled;
        int triggeredByUser;

        bool operator==(const CacheKey &other) const
        {
            return file == other.file
                && globalIndex == other.globalIndex
                && decodeEnabled == other.decodeEnabled
                && triggeredByUser == other.triggeredByUser;
        }

    };

    struct CacheKeyHash
    {
        std::size_t operator()(const CacheKey &key) const
        {
            const std::size_t h1 = std::hash<const void*>{}(static_cast<const void*>(key.file));
            const std::size_t h2 = std::hash<int>{}(key.globalIndex);
            const std::size_t h3 = std::hash<bool>{}(key.decodeEnabled);
            const std::size_t h4 = std::hash<int>{}(key.triggeredByUser);
            return (((h1 * 31U) + h2) * 31U + h3) * 31U + h4;
        }
    };

    static constexpr int kMaxEntries = 8192;

    std::mutex m_cacheLock;
    std::unordered_map<CacheKey, QDltMsg, CacheKeyHash> m_cache;
    std::deque<CacheKey> m_fifoOrder;

    //! Evict oldest entries until the cache is within limits.
    void pruneIfNeeded();
};

#endif // DECODECACHESERVICE_H