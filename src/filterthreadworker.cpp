#include "filterthreadworker.h"

#include "decodemanager.h"
#include "qdltpluginmanager.h"

FilterThreadBatchResult FilterThreadWorker::processBatch(const QVector<FilterThreadInputMessage> &messages,
                                                         QDltFilterList filterList,
                                                         bool filtersEnabled,
                                                         bool pluginsEnabled,
                                                         bool silentMode,
                                                         bool dltv2Support,
                                                         QDltPluginManager *pluginManager)
{
    FilterThreadBatchResult result;
    result.items.reserve(messages.size());
    result.matchingIndices.reserve(messages.size());

    for(const FilterThreadInputMessage &input : messages)
    {
        FilterThreadBatchResultItem item;
        item.index = input.index;

        if(input.rawMessage.isEmpty())
        {
            result.items.push_back(item);
            continue;
        }

        if(!item.decodedMsg.setMsg(input.rawMessage, true, dltv2Support))
        {
            result.items.push_back(item);
            continue;
        }

        item.decodedMsg.setIndex(input.index);
        DecodeManager::instance().decode(pluginManager, item.decodedMsg, pluginsEnabled, silentMode);
        item.passFilter = !filtersEnabled || filterList.checkFilter(item.decodedMsg);
        item.valid = true;

        if(item.passFilter)
        {
            result.matchingIndices.push_back(input.index);
        }

        result.items.push_back(item);
    }

    return result;
}
