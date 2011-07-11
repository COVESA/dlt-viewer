#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QLabel>
#include <QTimer>
#include <QSettings>

#include "treemodel.h"
#include "project.h"
#include "settingsdialog.h"
#include "qdlt.h"

extern "C"
{
        #include "dlt_common.h"
        #include "dlt_user_shared.h"
}

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString filename, QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:

    void on_tableView_customContextMenuRequested(QPoint pos);
    void on_markerWidget_customContextMenuRequested(QPoint pos);
    void on_nfilterWidget_customContextMenuRequested(QPoint pos);
    void on_filterWidget_itemSelectionChanged();
    void on_actionFilter_Delete_triggered();
    void on_actionFilter_Edit_triggered();
    void on_actionFilter_Add_triggered();
    void on_pfilterWidget_customContextMenuRequested(QPoint pos);
    void on_filterWidget_customContextMenuRequested(QPoint pos);
    void filterAdd();
    void filterAddTable();

    void on_actionPlugin_Hide_triggered();
    void on_actionPlugin_Show_triggered();
    void on_actionImport_DLT_Stream_with_Serial_Header_triggered();
    void on_actionExport_Selection_triggered();
    void on_actionAppend_DLT_File_triggered();
    void on_actionFilter_Clear_all_triggered();
    void on_configWidget_itemSelectionChanged();
    void on_pluginWidget_itemSelectionChanged();
    void autoscrollToggled(bool state);
    void filterToggled(bool state);
    void on_actionSearch_Continue_triggered();
    void on_actionSearch_triggered();
    void on_actionExport_ASCII_triggered();
    void on_actionImport_DLT_Stream_triggered();
    void on_action_Info_triggered();
    void on_actionECU_Delete_All_Contexts_triggered();
    void on_actionNew_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSend_Injection_triggered();
    void on_actionGet_Local_Time_triggered();
    void on_actionGet_Software_Version_triggered();
    void on_actionReset_to_Factory_Default_triggered();
    void on_actionStore_Config_triggered();
    void on_actionGet_Log_Info_triggered();
    void on_actionSet_All_Log_Levels_triggered();
    void on_actionSet_Log_Level_triggered();
    void on_actionSet_Default_Log_Level_triggered();
    void on_actionGet_Default_Log_Level_triggered();
    void on_actionECU_Disconnect_triggered();
    void on_actionECU_Connect_triggered();
    void on_actionPlugin_Edit_triggered();
    void on_pluginWidget_customContextMenuRequested(QPoint pos);
    void on_actionContext_Delete_triggered();
    void on_actionContext_Edit_triggered();
    void on_actionContext_Add_triggered();
    void on_actionApplication_Delete_triggered();
    void on_actionApplication_Edit_triggered();
    void on_actionApplication_Add_triggered();
    void on_configWidget_customContextMenuRequested(QPoint pos);
    void on_configWidget_itemClicked(QTreeWidgetItem* item, int column);
    void on_actionECU_Edit_triggered();
    void on_actionECU_Delete_triggered();
    void on_actionECU_Add_triggered();
    void on_actionProjectSave_triggered();
    void on_actionProjectOpen_triggered();
    void on_actionProjectNew_triggered();
    void on_tableView_clicked(QModelIndex index);
    void on_tableViewTriggerSelectionModel(QModelIndex indexNew,QModelIndex indexOld);
    void on_actionQuit_triggered();
    void on_actionSettings_triggered();
    void on_actionClear_triggered();
    void on_actionOpen_triggered();
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError);
    void readyRead ();
    void timeout();
    void timeoutRead();
    void connectAll();
    void disconnectAll();
    void applySettings();
    void updateScrollButton();
    void openRecentFile();
    void openRecentProject();
    void tableViewValueChanged(int value);
    void stateChanged(QAbstractSocket::SocketState socketState);
    void closeEvent(QCloseEvent *event);

public slots:

    void sendInjection(int index,QString applicationId,QString contextId,int serviceId,QByteArray data);

public:   
    /* Application settings */
    QSettings *bmwsettings;

    /* Project configuration containing ECU/APP/Context/Filter/Plugin configuration */
    Project project;

    /* DLT file handling */
    //DltFile file;
    QDltFile qfile;
    QDltControl qcontrol;
    QFile outputfile;
    TreeModel *tableModel;
    QMutex mutex;
    QString workingDirectory;

    /* Status line items */
    QLabel *statusFilename;
    QLabel *statusBytesReceived;
    QLabel *statusByteErrorsReceived;
    unsigned long totalBytesRcvd;
    unsigned long totalByteErrorsRcvd;

    /* search text */
    QString searchText;
    int searchLine;
    bool searchHeader;
    bool searchPayload;
    bool searchCaseSensitive;
    bool searchRegExp;

    /* Settings dialog containing also the settings parameter itself */
    SettingsDialog settings;

    /* injections */
    QString injectionAplicationId;
    QString injectionContextId;
    QString injectionServiceId;
    QString injectionData;

    /* Timer for connecting to ECUs */
    QTimer timer;
    QTimer timerRead;

    /* Toggle button */
    QPushButton *scrollbutton;
    QPushButton *filterbutton;

    /* Recent files */
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
    QStringList recentFiles;

    /* Recent projects */
    enum { MaxRecentProjects = 5 };
    QAction *recentProjectActs[MaxRecentProjects];
    QStringList recentProjects;

    /* Recent hostnames and ports */
    enum { MaxRecentHostnames = 10 };
    QStringList recentHostnames;
    enum { MaxRecentPorts = 10 };
    QStringList recentPorts;

    void getSelectedItems(EcuItem **ecuitem,ApplicationItem** appitem,ContextItem** conitem);

    void reloadLogFile();

    void ControlServiceRequest(EcuItem* ecuitem, int service_id );
    void SendInjection(EcuItem* ecuitem);

    void SendControlMessage(EcuItem* ecuitem,DltMessage &msg, QString appid, QString contid);
    void SetLogLevel(EcuItem* ecuitem, QString app, QString con,int log_level);
    void SetDefaultLogLevel(EcuItem* ecuitem, int status);
    void SetTraceStatus(EcuItem* ecuitem,QString app, QString con,int status);
    void SetDefaultTraceStatus(EcuItem* ecuitem, int status);
    void SetVerboseMode(EcuItem* ecuitem, int mode);
    void SetTimingPackets(EcuItem* ecuitem, bool enable);
    void GetLogInfo(EcuItem* ecuitem);

    void ReceiveControlMessage(EcuItem *ecuitem,DltMessage &msg);
    void SetContext(EcuItem *ecuitem, QString apid, QString ctid,QString ctdescription,int log_level,int trace_status);
    void SetApplication(EcuItem *ecuitem, QString apid, QString appdescription);

    void filterUpdate();

    void loadPlugins();
    void updatePluginsECUList();
    void updatePlugins();
    void updatePlugin(PluginItem *item);

    void connectECU(EcuItem *ecuitem,bool force = false);
    void disconnectECU(EcuItem *ecuitem);
    void read(EcuItem *ecuitem);

    void updateRecentFileActions();   
    void setCurrentFile(const QString &fileName);
    void removeCurrentFile(const QString &fileName);

    void updateRecentProjectActions();
    void setCurrentProject(const QString &projectName);
    void removeCurrentProject(const QString &projectName);

    void setCurrentHostname(const QString &hostName);
    void setCurrentPort(const QString &portName);

    void sendUpdates(EcuItem* ecuitem);
};

#endif // MAINWINDOW_H
