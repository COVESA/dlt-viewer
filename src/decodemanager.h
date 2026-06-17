#ifndef DECODEMANAGER_H
#define DECODEMANAGER_H

#include <QMutex>

class QDltPluginManager;
class QDltMsg;

/**
 * @brief Thread-safe gateway for plugin-based message decoding.
 */
class DecodeManager
{
public:
    /**
     * @brief Returns the singleton decode manager instance.
     * @return DecodeManager singleton reference.
     */
    static DecodeManager &instance();

    /**
     * @brief Decodes a message through the plugin manager.
     * @param pluginManager Plugin manager used for decode callbacks.
     * @param msg Message to decode in-place.
     * @param enabled True to run decoding, false to skip.
     * @param silentMode True for reduced decode-side verbosity.
     */
    void decode(QDltPluginManager *pluginManager, QDltMsg &msg, bool enabled, bool silentMode);

private:
    DecodeManager() = default;
    Q_DISABLE_COPY(DecodeManager)

    QMutex m_mutex;
};

#endif // DECODEMANAGER_H
