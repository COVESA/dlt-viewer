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

#include "treemodel.h"
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
    void iterateDecodersForMsg(QDltMsg &, int triggeredByUser);

protected:
    void keyPressEvent ( QKeyEvent * event );
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void on_pluginWidget_itemExpanded(QTreeWidgetItem* item);
    void on_actionFilter_Load_triggered();
    void on_actionFilter_Save_As_triggered();
    void on_actionExport_Selection_ASCII_triggered();
    void on_tableView_customContextMenuRequested(QPoint pos);
    void on_actionFilter_Delete_triggered();
    void on_actionFilter_Edit_triggered();
    void on_actionFilter_Add_triggered();
    void on_actionFilter_Duplicate_triggered();
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
    void on_actionFind_triggered();
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
    void on_actionContext_MultipleContextsEdit_triggered();
    void on_actionContext_Add_triggered();
    void on_actionApplication_Delete_triggered();
    void on_actionApplication_Edit_triggered();
    void on_actionApplication_Add_triggered();
    void on_configWidget_customContextMenuRequested(QPoint pos);
    void on_configWidget_itemClicked(QTreeWidgetItem* item, int column);
    QStringList getSerialPortsWithQextEnumartor();
    void on_actionECU_Edit_triggered();
    void on_actionECU_Delete_triggered();
    void on_actionECU_Add_triggered();
    void on_actionProjectSave_triggered();
    void on_actionProjectOpen_triggered();
    void on_actionProjectNew_triggered();
    void on_tableView_clicked(QModelIndex index);
    void tableViewTriggerSelectionModel(QModelIndex indexNew,QModelIndex indexOld);
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
    void openRecentFilters();
    void tableViewValueChanged(int value);
    void stateChangedTCP(QAbstractSocket::SocketState socketState);
    void stateChangedSerial(bool dsrChanged);
    void closeEvent(QCloseEvent *event);
    void sectionInTableDoubleClicked(int logicalIndex);

    void on_actionCommand_Line_triggered();

    void on_filterWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_actionCollapse_All_ECUs_triggered();

    void on_actionExpand_All_ECUs_triggered();

public slots:
    void sendInjection(int index,QString applicationId,QString contextId,int serviceId,QByteArray data);

public:   

    /* Command line manager */
    OptManager *optManager;

    /* Application settings */
    DltSettingsManager *bmwsettings;

    /* Project configuration containing ECU/APP/Context/Filter/Plugin configuration */
    Project project;

    /* Timer for connecting to ECUs */
    QTimer timer;
    QTimer timerRead;

    /* DLT file handling */
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

    /* Search */
    SearchDialog *searchDlg;

    /* Settings dialog containing also the settings parameter itself */
    SettingsDialog settings;
    QLineEdit *searchTextToolbar;

    /* injections */
    QString injectionAplicationId;
    QString injectionContextId;
    QString injectionServiceId;
    QString injectionData;

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

    void logfileOpen(QString fileName);
    bool projectfileOpen(QString filename);



};

#endif // MAINWINDOW_H
