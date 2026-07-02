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
 * \file messagestore.h
 * @licence end@
 */

#ifndef MESSAGESTORE_H
#define MESSAGESTORE_H

#include "export_rules.h"
#include "qdltmsg.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

class QDltFile;

using MessageId = std::uint64_t;
static constexpr MessageId kInvalidMessageId = (std::numeric_limits<MessageId>::max)();

class QDLT_EXPORT CMessageStore
{
public:
    //! Virtual destructor for message store implementations.
    virtual ~CMessageStore() = default;

    //! Return total number of messages available in the store.
    virtual std::size_t size() const = 0;
    //! Check whether a message id exists in the store.
    virtual bool contains(MessageId messageId) const = 0;

    //! Resolve a global index to a stable message id.
    virtual MessageId messageIdForGlobalIndex(int index) const = 0;
    //! Resolve a filtered row to a stable message id.
    virtual MessageId messageIdForFilteredRow(int row) const = 0;
    //! Resolve a stable message id to its global index.
    virtual int globalIndexForMessageId(MessageId messageId) const = 0;

    //! Return the raw serialized message payload bytes for a message id.
    virtual std::vector<char> rawMessage(MessageId messageId) const = 0;
    //! Decode a message by id into QDltMsg.
    virtual bool message(MessageId messageId, QDltMsg &msg) const = 0;

    //! Return a non-owning view of all message ids currently available.
    virtual const std::vector<MessageId> &snapshotAllMessageIds() const = 0;
    //! Return a non-owning view of message ids currently visible in the filtered view.
    virtual const std::vector<MessageId> &snapshotFilteredMessageIds() const = 0;
};

class QDLT_EXPORT CQDltFileMessageStoreAdapter final : public CMessageStore
{
public:
    //! Create an adapter bound to an optional QDltFile.
    explicit CQDltFileMessageStoreAdapter(const QDltFile *file = nullptr);

    //! Update the bound file instance used for message queries.
    void setFile(const QDltFile *file);
    //! Return the currently bound file instance.
    const QDltFile *file() const;

    //! Return total number of messages available in the store.
    std::size_t size() const override;
    //! Check whether a message id exists in the store.
    bool contains(MessageId messageId) const override;

    //! Resolve a global index to a stable message id.
    MessageId messageIdForGlobalIndex(int index) const override;
    //! Resolve a filtered row to a stable message id.
    MessageId messageIdForFilteredRow(int row) const override;
    //! Resolve a stable message id to its global index.
    int globalIndexForMessageId(MessageId messageId) const override;

    //! Return the raw serialized message payload bytes for a message id.
    std::vector<char> rawMessage(MessageId messageId) const override;
    //! Decode a message by id into QDltMsg.
    bool message(MessageId messageId, QDltMsg &msg) const override;

    //! Return a non-owning view of all message ids currently available.
    const std::vector<MessageId> &snapshotAllMessageIds() const override;
    //! Return a non-owning view of message ids currently visible in the filtered view.
    const std::vector<MessageId> &snapshotFilteredMessageIds() const override;

private:
    const QDltFile *m_file;
    mutable std::vector<MessageId> m_allMessageIdsCache;
    mutable std::vector<MessageId> m_filteredMessageIdsCache;
};

#endif // MESSAGESTORE_H
