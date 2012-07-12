/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file mainwindow.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QLabel>
#include <QTimer>
#include <QDir>

#include "tablemodel.h"
#include "project.h"
#include "settingsdialog.h"
#include "searchdialog.h"
#include "optmanager.h"
#include "qdlt.h"
#include "dltsettingsmanager.h"

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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    Ui::MainWindow *ui;

    void commandLineConvertToASCII();
    void commandLineExecutePlugin(QString plugin, QString cmd, QStringList params);
    void iterateDecodersForMsg(QDltMsg &, int triggeredByUser);
    QStringList getSerialPortsWithQextEnumartor();

protected:
    void keyPressEvent ( QKeyEvent * event );
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_tableView_clicked(QModelIndex index);

    void on_tableView_customContextMenuRequested(QPoint pos);
    void on_pluginWidget_customContextMenuRequested(QPoint pos);
    void on_filterWidget_customContextMenuRequested(QPoint pos);
    void on_configWidget_customContextMenuRequested(QPoint pos);

    void on_configWidget_itemSelectionChanged();
    void on_pluginWidget_itemSelectionChanged();
    void on_filterWidget_itemSelectionChanged();

    void on_filterWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_pluginWidget_itemExpanded(QTreeWidgetItem* item);


    // File methods
    void on_action_menuFile_New_triggered();
    void on_action_menuFile_SaveAs_triggered();
    void on_action_menuFile_Import_DLT_Stream_with_Serial_Header_triggered();
    void on_action_menuFile_Export_Selection_triggered();
    void on_action_menuFile_Append_DLT_File_triggered();
    void on_action_menuFile_Export_Selection_ASCII_triggered();
    void on_action_menuFile_Export_ASCII_triggered();
    void on_action_menuFile_Import_DLT_Stream_triggered();
    void on_action_menuFile_Quit_triggered();
    void on_action_menuFile_Settings_triggered();
    void on_action_menuFile_Clear_triggered();
    void on_action_menuFile_Open_triggered();

    // Search methods
    void on_action_menuSearch_Find_triggered();

    // Project methods
    void on_action_menuProject_Save_triggered();
    void on_action_menuProject_Open_triggered();
    void on_action_menuProject_New_triggered();

    // Help methods
    void on_action_menuHelp_Info_triggered();
    void on_action_menuHelp_Command_Line_triggered();

    // Config methods
    void on_action_menuConfig_Context_Delete_triggered();
    void on_action_menuConfig_Context_Edit_triggered();
    void on_action_menuConfig_Context_Add_triggered();
    void on_action_menuConfig_Application_Delete_triggered();
    void on_action_menuConfig_Application_Edit_triggered();
    void on_action_menuConfig_Application_Add_triggered();
    void on_action_menuConfig_ECU_Edit_triggered();
    void on_action_menuConfig_ECU_Delete_triggered();
    void on_action_menuConfig_ECU_Add_triggered();
    void on_action_menuConfig_Collapse_All_ECUs_triggered();
    void on_action_menuConfig_Expand_All_ECUs_triggered();
    void on_action_menuConfig_Disconnect_triggered();
    void on_action_menuConfig_Connect_triggered();
    void on_action_menuConfig_Delete_All_Contexts_triggered();
    void on_action_menuConfig_Copy_to_clipboard_triggered();

    // DLT methods
    void on_action_menuDLT_Send_Injection_triggered();
    void on_action_menuDLT_Get_Local_Time_triggered();
    void on_action_menuDLT_Get_Software_Version_triggered();
    void on_action_menuDLT_Reset_to_Factory_Default_triggered();
    void on_action_menuDLT_Store_Config_triggered();
    void on_action_menuDLT_Set_All_Log_Levels_triggered();
    void on_action_menuDLT_Set_Log_Level_triggered();
    void on_action_menuDLT_Set_Default_Log_Level_triggered();
    void on_action_menuDLT_Get_Default_Log_Level_triggered();
    void on_action_menuDLT_Get_Log_Info_triggered();
    void on_action_menuDLT_Edit_All_Log_Levels_triggered();

    // Filter methods
    void on_action_menuFilter_Load_triggered();
    void on_action_menuFilter_Save_As_triggered();
    void on_action_menuFilter_Delete_triggered();
    void on_action_menuFilter_Edit_triggered();
    void on_action_menuFilter_Add_triggered();
    void on_action_menuFilter_Clear_all_triggered();
    void on_action_menuFilter_Duplicate_triggered();
    void on_action_menuFilter_Append_Filters_triggered();

    // Plugin methods
    void on_action_menuPlugin_Hide_triggered();
    void on_action_menuPlugin_Show_triggered();
    void on_action_menuPlugin_Edit_triggered();
    void on_action_menuPlugin_Disable_triggered();
    void on_action_menuPlugin_ExecuteCommand_triggered();

    //Rename
    void filterAdd();
    void filterAddTable();
    void autoscrollToggled(bool state);
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError);
    void readyRead();
    void timeout();
    void timeoutRead();
    void connectAll();
    void disconnectAll();
    void applySettings();
    void updateScrollButton();
    void openRecentFile();
    void openRecentProject();
    void openRecentFilters();
    void tableViewValueChanged(int value);
    void stateChangedTCP(QAbstractSocket::SocketState socketState);
    void stateChangedSerial(bool dsrChanged);
    void sectionInTableDoubleClicked(int logicalIndex);
    void on_filterButton_clicked(bool checked);





public slots:
    void sendInjection(int index,QString applicationId,QString contextId,int serviceId,QByteArray data);

public:   

    /* Project configuration containing ECU/APP/Context/Filter/Plugin configuration */
    Project project;

    /* Timer for connecting to ECUs */
    QTimer timer;
    QTimer timerRead;

    /* DLT file handling */
    QDltFile qfile;
    QDltControl qcontrol;
    QFile outputfile;
    TableModel *tableModel;
    QMutex mutex;
    QString workingDirectory;

    /* Status line items */
    QLabel *statusFilename;
    QLabel *statusBytesReceived;
    QLabel *statusByteErrorsReceived;
    unsigned long totalBytesRcvd;
    unsigned long totalByteErrorsRcvd;

    /* Search */
    SearchDialog *searchDlg;

    /* Settings dialog containing also the settings parameter itself */
    SettingsDialog *settings;
    QLineEdit *searchTextToolbar;

    /* injections */
    QString injectionAplicationId;
    QString injectionContextId;
    QString injectionServiceId;
    QString injectionData;

    /* Toggle button */
    QPushButton *scrollbutton;

    /* Recent files */
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
    QStringList recentFiles;

    /* Recent projects */
    enum { MaxRecentProjects = 5 };
    QAction *recentProjectActs[MaxRecentProjects];
    QStringList recentProjects;

    /* Recent filters */
    enum { MaxRecentFilters = 5 };
    QAction *recentFiltersActs[MaxRecentFilters];
    QStringList recentFilters;

    /* Recent hostnames and ports */
    enum { MaxRecentHostnames = 10 };
    QStringList recentHostnames;
    enum { MaxRecentPorts = 10 };
    QStringList recentPorts;



    void getSelectedItems(EcuItem **ecuitem,ApplicationItem** appitem,ContextItem** conitem);

    void reloadLogFile();

    void exportSelection(bool ascii,bool file);

    void ControlServiceRequest(EcuItem* ecuitem, int service_id );
    void SendInjection(EcuItem* ecuitem);

    void controlMessage_SendControlMessage(EcuItem* ecuitem,DltMessage &msg, QString appid, QString contid);
    void controlMessage_SetLogLevel(EcuItem* ecuitem, QString app, QString con,int log_level);
    void controlMessage_SetDefaultLogLevel(EcuItem* ecuitem, int status);
    void controlMessage_SetTraceStatus(EcuItem* ecuitem,QString app, QString con,int status);
    void controlMessage_SetDefaultTraceStatus(EcuItem* ecuitem, int status);
    void controlMessage_SetVerboseMode(EcuItem* ecuitem, int mode);
    void controlMessage_SetTimingPackets(EcuItem* ecuitem, bool enable);
    void controlMessage_GetLogInfo(EcuItem* ecuitem);
    void controlMessage_ReceiveControlMessage(EcuItem *ecuitem,DltMessage &msg);
    void controlMessage_SetContext(EcuItem *ecuitem, QString apid, QString ctid,QString ctdescription,int log_level,int trace_status);
    void controlMessage_SetApplication(EcuItem *ecuitem, QString apid, QString appdescription);

    void filterUpdate();

    void loadPlugins();
    void loadPluginsPath(QDir dir);
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

    void updateRecentFiltersActions();
    void setCurrentFilters(const QString &filtersName);
    void removeCurrentFilters(const QString &filtersName);

    void setCurrentHostname(const QString &hostName);
    void setCurrentPort(const QString &portName);

    void sendUpdates(EcuItem* ecuitem);

    void openDltFile(QString fileName);
    bool openDlpFile(QString filename);



};

#endif // MAINWINDOW_H
