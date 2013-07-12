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
 * \file settingsdialog.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMainWindow>
#include "qdlt.h"

#define DEFAULT_REFRESH_RATE 20

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QDltFile *_qFile, QWidget *parent = 0);
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    int     tempUseSystem;
    QString tempSystemPath;
    int     tempUseOwn;
    QString tempOwnPath;
    int     tempCloseWithoutAsking;
    int     tempSaveOnClear;


    int defaultLogFile;
    QString defaultLogFileName;
    int defaultProjectFile;
    QString defaultProjectFileName;
    int pluginsPath;
    QString pluginsPathName;
    int defaultFilterPath;
    QString defaultFilterPathName;

    int autoConnect;
    int autoScroll;
    int autoMarkFatalError;
    int autoMarkWarn;
    int writeControl;
    int updateContextLoadingFile;

    int fontSize;
    int showIndex;
    int showTime;
    int showTimestamp;
    int showCount;
    int showEcuId;
    int showApId;
    int showApIdDesc;
    int showCtId;
    int showCtIdDesc;
    int showType;
    int showSubtype;
    int showMode;
    int showNoar;
    int showPayload;

    int automaticTimeSettings;
    qlonglong utcOffset;
    int dst;

    QString workingDirectory;

    void assertSettingsVersion();

    void writeDlg();
    void readDlg();

    void writeSettings(QMainWindow *mainwindow);
    void readSettings();

    void resetSettings();

    QStringList getRecentFiles();
    QStringList getRecentProjects();
    QStringList getRecentFilters();
    QString getWorkingDirectory();

protected:
    void changeEvent(QEvent *e);

private:
    QDltFile *qFile;
    Ui::SettingsDialog *ui;


private slots:
    void on_groupBoxAppId_clicked(bool checked);
    void on_groupBoxConId_clicked(bool checked);
    void on_groupBoxAutomaticTimeSettings_clicked(bool checked);
    void on_tooButtonPluginsPath_clicked();
    void on_toolButtonDefaultProjectFile_clicked();
    void on_toolButtonDefaultLogFile_clicked();
    void on_toolButtonTempPath_clicked();

    void on_toolButtonDefaultFilterPath_clicked();
};

#endif // SETTINGSDIALOG_H
