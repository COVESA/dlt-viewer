#include "decodemanager.h"

#include "qdltplugin.h"
#include "qdltpluginmanager.h"

#include <QMutexLocker>

DecodeManager::DecodeManager(QDltPluginManager *pluginManager)
    : pluginManager(pluginManager)
{
}

void DecodeManager::refreshDecoderChain()
{
    if(pluginManager == nullptr)
    {
        return;
    }

    const QList<QDltPlugin*> snapshot = pluginManager->getDecoderPlugins();
    QMutexLocker locker(&decoderChainMutex);
    decoderPlugins = snapshot;
}

void DecodeManager::decodeMsg(QDltMsg &msg, int triggeredByUser)
{
    QList<QDltPlugin*> snapshot;
    {
        QMutexLocker locker(&decoderChainMutex);
        snapshot = decoderPlugins;
    }

    if(pluginManager != nullptr)
    {
        pluginManager->decodeMsgUsingPlugins(snapshot, msg, triggeredByUser);
        return;
    }

    for(int i = 0; i < snapshot.size(); ++i)
    {
        QDltPlugin *plugin = snapshot.at(i);
        if(plugin != nullptr && plugin->decodeMsg(msg, triggeredByUser))
        {
            break;
        }
    }
}
