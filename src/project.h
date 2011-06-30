#ifndef PROJECT_H
#define PROJECT_H

#include "qdlt.h"

#include <QTreeWidgetItem>
#include <QTcpSocket>
#include <QDockWidget>
#include <qextserialport.h>

extern "C"
{
        #include "dlt_common.h"
        #include "dlt_user_shared.h"
}

#include "plugininterface.h"

#define DLT_VIEWER_BUFFER_SIZE 256000
#define RCVBUFSIZE 128000   /* Size of receive buffer */

enum dlt_item_type { ecu_type = QTreeWidgetItem::UserType, application_type, context_type, filter_type, plugin_type };

class EcuItem  : public QTreeWidgetItem
{
public:

    EcuItem(QTreeWidgetItem *parent = 0);
    ~EcuItem();

    void InvalidAll();

    /* configuration all */
    QString id;
    QString description;
    int interfacetype;
    int loglevel;
    int tracestatus;
    int verbosemode;
    bool timingPackets;
    bool sendGetLogInfo;
    enum {invalid,unknown,valid} status;

   /* configuration TCP */
    QString hostname;
    unsigned int tcpport;
    bool sendSerialHeaderTcp;
    bool syncSerialHeaderTcp;

    QDltTCPConnection tcpcon;

    /* configuration serial */
    QString port;
    int baudrate;
    bool sendSerialHeaderSerial;
    bool syncSerialHeaderSerial;

    QDltSerialConnection serialcon;

    /* configuration update */
    bool updateDataIfOnline;
    void update();

    /* connection */
    QTcpSocket socket;
    //AbstractSerial *serialport;
    QextSerialPort *serialport;

    /* connection status */
    int tryToConnect;
    int connected;
    QString connectError;

    /* current received message and buffer for receivig from the socket */
    int32_t totalBytesRcvd;
    QByteArray data;

private:


};

class ApplicationItem  : public QTreeWidgetItem
{
public:

    ApplicationItem(QTreeWidgetItem *parent = 0);
    ~ApplicationItem();

    QString id;
    QString description;

    void update();

private:

};

class ContextItem  : public QTreeWidgetItem
{
public:

    ContextItem(QTreeWidgetItem *parent = 0);
    ~ContextItem();

    enum {invalid,unknown,valid} status;

    QString id;
    QString description;
    int loglevel;
    int tracestatus;

    void update();

private:

};

class FilterItem  : public QTreeWidgetItem
{
public:

    FilterItem(QTreeWidgetItem *parent = 0);
    ~FilterItem();

    QString ecuId;
    QString applicationId;
    QString contextId;
    QString headerText;
    QString payloadText;

    bool enableEcuId;
    bool enableApplicationId;
    bool enableContextId;
    bool enableHeaderText;
    bool enablePayloadText;

    int filterColour;

    void update();

private:

};

class PluginItem  : public QTreeWidgetItem
{
public:

    PluginItem(QTreeWidgetItem *parent = 0);
    ~PluginItem();

    enum { ModeDisable, ModeEnable, ModeShow };
    enum { TypeFile, TypeDirectory };

    QString name;
    QString filename;
    int mode;
    int type;

    void update();

    QDLTPluginInterface *plugininterface;
    QDLTPluginDecoderInterface *plugindecoderinterface;
    QDltPluginViewerInterface  *pluginviewerinterface;
    QWidget *widget;
    QDockWidget *dockWidget;

private:

};

class Project
{
public:

    Project();
    ~Project();

    bool Load(QString filename);
    bool Save(QString filename);
    void Clear();

    QTreeWidget *ecu;
    QTreeWidget *pfilter;
    QTreeWidget *nfilter;
    QTreeWidget *marker;
    QTreeWidget *plugin;

private:


};

#endif // PROJECT_H
