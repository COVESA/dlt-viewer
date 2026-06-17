#ifndef FILTERTHREADWORKER_H
#define FILTERTHREADWORKER_H

#include "qdltfilterlist.h"

#include <QByteArray>
#include <QVector>

class QDltPluginManager;

/**
 * @brief Input payload for one message in a filter worker batch.
 */
struct FilterThreadInputMessage
{
    int index{-1};
    QByteArray rawMessage;
};

/**
 * @brief Processing result for one message in a filter worker batch.
 */
struct FilterThreadBatchResultItem
{
    int index{-1};
    bool passFilter{false};
    bool valid{false};
    QDltMsg decodedMsg;
};

/**
 * @brief Aggregated processing output for one filter worker batch.
 */
struct FilterThreadBatchResult
{
    QVector<FilterThreadBatchResultItem> items;
    QVector<qint64> matchingIndices;
};

/**
 * @brief Stateless helper for batch decoding and filter evaluation.
 */
class FilterThreadWorker
{
public:
    /**
     * @brief Processes a message batch and returns decode/filter results.
     * @param messages Raw input messages with target indices.
     * @param filterList Active filter list snapshot.
     * @param filtersEnabled True to apply filter checks.
     * @param pluginsEnabled True to run plugin decoding.
     * @param silentMode True to suppress decode verbosity.
     * @param dltv2Support True to parse DLTv2 payloads.
     * @param pluginManager Plugin manager used for decoding.
     * @return Batch result containing per-item output and matching indices.
     */
    static FilterThreadBatchResult processBatch(const QVector<FilterThreadInputMessage> &messages,
                                                QDltFilterList filterList,
                                                bool filtersEnabled,
                                                bool pluginsEnabled,
                                                bool silentMode,
                                                bool dltv2Support,
                                                QDltPluginManager *pluginManager);
};

#endif // FILTERTHREADWORKER_H
