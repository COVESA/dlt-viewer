#include "decodemanager.h"

#include "qdltpluginmanager.h"

#include <QMutexLocker>

DecodeManager &DecodeManager::instance()
{
    static DecodeManager manager;
    return manager;
}

void DecodeManager::decode(QDltPluginManager *pluginManager, QDltMsg &msg, bool enabled, bool silentMode)
{
    if(!enabled || pluginManager == nullptr)
    {
        return;
    }

    QMutexLocker lock(&m_mutex);
    pluginManager->decodeMsg(msg, silentMode);
}
