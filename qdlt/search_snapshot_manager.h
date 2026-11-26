#ifndef QDLT_SEARCH_SNAPSHOT_MANAGER_H
#define QDLT_SEARCH_SNAPSHOT_MANAGER_H

#include <QMutex>

#include "qdlt/export_rules.h"
#include "qdlt/qdltfile.h"
#include "qdlt/searchsnapshot.h"

#include <memory>

class QDLT_EXPORT SearchSnapshotManager {
public:
    std::shared_ptr<const SearchSnapshot> capture(const QDltFile* file);

private:
    mutable QMutex m_mutex;
    const QDltFile* m_cachedFile{nullptr};
    std::shared_ptr<const SearchSnapshot> m_cachedSnapshot;
};

#endif // QDLT_SEARCH_SNAPSHOT_MANAGER_H
