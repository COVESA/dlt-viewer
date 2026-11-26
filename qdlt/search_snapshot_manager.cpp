#include "qdlt/search_snapshot_manager.h"

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
