#ifndef DUMMYVIEWERPLUGIN_H
#define DUMMYVIEWERPLUGIN_H

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
    int counterMessages;
    int counterNonVerboseMessages;
    int counterVerboseMessages;

    void show(bool value);

    void updateCounters(int start,int end);

private:
    QDltFile *dltFile;
    QString errorText;
};

#endif // DUMMYVIEWERPLUGIN_H
