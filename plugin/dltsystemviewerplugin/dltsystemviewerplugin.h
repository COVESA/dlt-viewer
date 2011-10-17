#ifndef DLTSYSTEMVIEWERPLUGIN_H
#define DLTSYSTEMVIEWERPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"

#define DLT_SYSTEM_VIEWER_PLUGIN_VERSION "1.0.0"

class DltSystemViewerPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)

public:
    DltSystemViewerPlugin();
    ~DltSystemViewerPlugin();

    /* QDLTPluginInterface interface */
    QString name();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString description();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginViewerInterface */
    QWidget* initViewer();
    bool initFile(QDltFile *file);
    void updateFile();
    void selectedIdxMsg(int index);

    /* internal variables */
    Form *form;
    int counterMessages;
    int counterNonVerboseMessages;
    int counterVerboseMessages;

    int lastValueUser;
    int lastValueNice;
    int lastValueKernel;
    unsigned int lastTimeStamp;

    void show(bool value);

    void updateProcesses(int start,int end);

private:
    QDltFile *dltFile;
    QString errorText;
};

#endif // DLTSYSTEMVIEWERPLUGIN_H
