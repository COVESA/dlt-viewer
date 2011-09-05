#ifndef DUMMYDECODERPLUGIN_H
#define DUMMYDECODERPLUGIN_H

#include <QObject>
#include "plugininterface.h"

#define DUMMY_DECODER_PLUGIN_VERSION "1.0.0"

class DummyDecoderPlugin : public QObject, QDLTPluginInterface, QDLTPluginDecoderInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDLTPluginDecoderInterface)

public:
    DummyDecoderPlugin();
    ~DummyDecoderPlugin();

    /* QDLTPluginInterface interface */
    QString name();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString description();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginDecoderInterface */
    bool isMsg(QDltMsg &msg);
    bool decodeMsg(QDltMsg &msg);

private:
    QString errorText;
};

#endif // DUMMYDECODERPLUGIN_H
