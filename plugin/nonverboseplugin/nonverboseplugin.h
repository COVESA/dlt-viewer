#ifndef NONVERBOSEPLUGIN_H
#define NONVERBOSEPLUGIN_H

#include <QObject>
#include "nonverboseplugin.h"
#include "plugininterface.h"

#define NON_VERBOSE_PLUGIN_VERSION "1.0.0"

/**
 * The structure of a Fibex PDU information.
 */
class DltFibexPdu
{
public:
    DltFibexPdu() { byteLength=0;typeInfo=0; }

        QString id;
        QString description;
        int32_t byteLength;
        uint32_t typeInfo;
 };

 class DltFibexPduRef
 {
 public:
        DltFibexPduRef() { ref = 0; }

        QString id;
        DltFibexPdu *ref;
 };

 /**
 * The structure of a Fibex Frame information.
 */
class DltFibexFrame
{
public:
    DltFibexFrame() { byteLength=0;messageType=0;messageInfo=0;pduRefCounter=0; }

        QString id;
        int32_t byteLength;
        uint8_t messageType;
        int8_t  messageInfo;
        QString appid;
        QString ctid;

        QList<DltFibexPduRef*> pdureflist;
        uint32_t pduRefCounter;
};

class NonverbosePlugin : public QObject, QDLTPluginInterface, QDLTPluginDecoderInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDLTPluginDecoderInterface)

public:
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

    QList<DltFibexPdu*> pdulist;
    QList<DltFibexFrame*> framelist;

};

#endif // NONVERBOSEPLUGIN_H
