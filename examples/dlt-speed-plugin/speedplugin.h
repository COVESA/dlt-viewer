#ifndef SPEEDPLUGIN_H
#define SPEEDPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"

class SpeedPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)

public:
    SpeedPlugin();
    ~SpeedPlugin();

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

    void show(bool value);

private:
    QDltFile *dltFile;
    QString errorText;

    int msgIndex;
};

#endif // SPEEDRPLUGIN_H
