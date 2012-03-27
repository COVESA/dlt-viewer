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

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    int defaultLogFile;
    QString defaultLogFileName;
    int defaultProjectFile;
    QString defaultProjectFileName;
    int pluginsPath;
    QString pluginsPathName;

    int autoConnect;
    int autoScroll;
    int autoMarkFatalError;
    int autoMarkWarn;
    int writeControl;
    int hideFiletransfer;

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

    QString workingDirectory;

    void assertSettingsVersion();

    void writeDlg();
    void readDlg();

    void writeSettings();
    void readSettings();

    void resetSettings();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SettingsDialog *ui;

private slots:
    void on_groupBoxAppId_clicked(bool checked);
    void on_groupBoxConId_clicked(bool checked);
    void on_tooButtonPluginsPath_clicked();
    void on_toolButtonDefaultProjectFile_clicked();
    void on_toolButtonDefaultLogFile_clicked();
};

#endif // SETTINGSDIALOG_H
