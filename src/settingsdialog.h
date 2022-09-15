/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file settingsdialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMainWindow>
#include <QColorDialog>
#include "qdlt.h"

#define AUTOCONNECT_DEFAULT_TIME 1000 // in ms

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QDltFile *_qFile, QWidget *parent = 0);
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    QString workingDirectory;
    QFont font;

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

Q_SIGNALS:
    void FilterPathChanged();
    void PluginsAutoloadChanged();

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
    void on_toolButtonPluginsAutoload_clicked();
    void on_groupBoxSessionId_clicked(bool checked);
    void on_groupBoxArguments_clicked(bool checked);
    void on_spinBox_showArguments_valueChanged(int i);

    void on_checkBoxPluginsAutoload_stateChanged(int arg1);
    void on_pushButtonMarkerColor_clicked();
    void on_pushButtonSelectFont_clicked();
};

#endif // SETTINGSDIALOG_H
