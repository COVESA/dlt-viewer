#ifndef PROJECT_H
#define PROJECT_H

#include "qdlt.h"

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QTcpSocket>
#include <QDockWidget>
#include <QObject>
#include <qextserialport.h>
#include "settingsdialog.h"

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
    bool operator< ( const QTreeWidgetItem & other ) const;
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
    bool operator< ( const QTreeWidgetItem & other ) const;
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
    bool operator< ( const QTreeWidgetItem & other ) const;
};

class FilterItem  : public QTreeWidgetItem
{
public:

    FilterItem(QTreeWidgetItem *parent = 0);
    ~FilterItem();

    void operator = (FilterItem &item);

    typedef enum { positive = 0, negative, marker } FilterType;

    FilterType type;

    QString name;
    QString ecuId;
    QString applicationId;
    QString contextId;
    QString headerText;
    QString payloadText;

    bool enableFilter;
    bool enableEcuId;
    bool enableApplicationId;
    bool enableContextId;
    bool enableHeaderText;
    bool enablePayloadText;
    bool enableLogLevelMin;
    bool enableLogLevelMax;
    bool enableCtrlMsgs;

    QColor filterColour;

    int logLevelMax;
    int logLevelMin;

    void update();

private:

};

class PluginItem  : public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:

    PluginItem(QTreeWidgetItem *parent = 0);
    ~PluginItem();

    enum { ModeDisable, ModeEnable, ModeShow };
    enum { TypeFile, TypeDirectory };

    QString name;
    QString pluginVersion;
    QString pluginInterfaceVersion;
    QString filename;
    int mode;
    int type;

    void update();

    QDLTPluginInterface *plugininterface;
    QDLTPluginDecoderInterface *plugindecoderinterface;
    QDltPluginViewerInterface  *pluginviewerinterface;
    QDltPluginControlInterface *plugincontrolinterface;
    QWidget *widget;
    QDockWidget *dockWidget;

private:

public slots:
    void dockVisibilityChanged(bool);

};

class Project
{
public:

    Project();
    ~Project();

    bool Load(QString filename);
    bool Save(QString filename);
    void Clear();

    bool SaveFilter(QString filename);
    bool LoadFilter(QString filename);

    QTreeWidget *ecu;
    QTreeWidget *filter;
    QTreeWidget *plugin;
    SettingsDialog *settings;

private:


};

#endif // PROJECT_H
