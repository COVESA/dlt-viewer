#ifndef DLTVIEWERPLUGIN_H
#define DLTVIEWERPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"

class DltViewerPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)

public:
    DltViewerPlugin();
    ~DltViewerPlugin();

    /* QDLTPluginInterface interface */
    QString name();
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

    //void show(bool value);


private:

    QDltFile *dltFile;
    QString errorText;
};

#endif // DLTVIEWERPLUGIN_H
