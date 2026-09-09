#ifndef DECODEMANAGER_H
#define DECODEMANAGER_H

#include "qdltmessagedecoder.h"

#include <QList>
#include <QMutex>

class QDltPlugin;
class QDltPluginManager;

class DecodeManager : public QDltMessageDecoder
{
public:
    explicit DecodeManager(QDltPluginManager *pluginManager);

    void refreshDecoderChain();
    void decodeMsg(QDltMsg &msg, int triggeredByUser) override;

private:
    QDltPluginManager *pluginManager;
    QMutex decoderChainMutex;
    QList<QDltPlugin*> decoderPlugins;
};

#endif // DECODEMANAGER_H
