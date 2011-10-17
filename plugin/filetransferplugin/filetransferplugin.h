#ifndef DLTVIEWERPLUGIN_H
#define DLTVIEWERPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"
#include "globals.h"


#define FILETRANSFER_PLUGIN_VERSION "1.0.0"

class FiletransferPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)

public:
    FiletransferPlugin();
    ~FiletransferPlugin();

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
    void show(bool value);

private:
    Form *form;
    QDltFile *dltFile;
    QString errorText;
    int msgIndex;

    void doFLST(QDltMsg *msg);
    void doFLDA(int index, QDltMsg *msg);
    void doFLFI(QDltMsg *msg);
    void doFLIF(QDltMsg *msg);
    void doFLER(QDltMsg *msg);
};

#endif // DLTVIEWERPLUGIN_H
