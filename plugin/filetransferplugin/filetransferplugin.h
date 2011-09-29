#ifndef DLTVIEWERPLUGIN_H
#define DLTVIEWERPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"
#include "globals.h"
#include "filemodel.h"

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

    /* internal variables */
    Form *form;

    void show(bool value);


private:

    QDltFile *dltFile;
    QString errorText;
    FileModel *fModel;
    int msgIndex;
};

#endif // DLTVIEWERPLUGIN_H
