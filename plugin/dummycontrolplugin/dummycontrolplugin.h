#ifndef DUMMYCONTROLPLUGIN_H
#define DUMMYCONTROLPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"

class DummyControlPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)

public:
    DummyControlPlugin();
    ~DummyControlPlugin();

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

    /* QDltPluginControlInterface */
    bool initControl(QDltControl *control);
    bool initConnections(QStringList list);
    bool controlMsg(int index, QDltMsg &msg);

    /* internal variables */
    Form *form;
    int counterMessages;
    int counterNonVerboseMessages;
    int counterVerboseMessages;

    void show(bool value);

    void updateCounters(int start,int end);

    QDltControl *dltControl;

private:
    QDltFile *dltFile;
    QString errorText;
};

#endif // DUMMYCONTROLPLUGIN_H
