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
 * \file settingsdialog.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QSettings>
#include <QFileDialog>
#include <qmessagebox.h>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "version.h"
#include "dltsettingsmanager.h"

SettingsDialog::SettingsDialog(QDltFile *_qFile, QWidget *parent):

    QDialog(parent), qFile(_qFile),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    //See man pages - tzset(3)
    //It is used to access DST and other timezone related information
    //and sets all timezone global variables
    tzset();

    //timezone contains the difference in seconds between UTC and the local
    //standard time
    //qDebug()<< "Difference between UTC and the local standard time: "<<timezone<< " seconds."<<endl;

    //daylight is a Boolean flag
    //qDebug()<<"Is DST in effect? "<<(daylight == 0 ? false:true)<<endl;

    //tzname[0] contains a textual timezone codename which represents the
    //local standard time
    //qDebug()<<"Local standard timezone code is:  "<<tzname[0]<<endl;

    //tzname[1] is the name of the local alternate timezone
    //qDebug()<<"Local alternate timezone code is: "<<tzname[1]<<endl;

    /* List of official UTC time offsets */
    ui->comboBoxUTCOffset->addItem("UTC-12:00",-12*3600);
    ui->comboBoxUTCOffset->addItem("UTC-11:00",-11*3600);
    ui->comboBoxUTCOffset->addItem("UTC-10:00",-10*3600);
    ui->comboBoxUTCOffset->addItem("UTC-09:30",-9*3600-30*60);
    ui->comboBoxUTCOffset->addItem("UTC-09:00",-9*3600);
    ui->comboBoxUTCOffset->addItem("UTC-08:00",-8*3600);
    ui->comboBoxUTCOffset->addItem("UTC-07:00",-7*3600);
    ui->comboBoxUTCOffset->addItem("UTC-06:00",-6*3600);
    ui->comboBoxUTCOffset->addItem("UTC-05:00",-5*3600);
    ui->comboBoxUTCOffset->addItem("UTC-04:30",-4*3600-30*60);
    ui->comboBoxUTCOffset->addItem("UTC-04:00",-4*3600);
    ui->comboBoxUTCOffset->addItem("UTC-03:30",-3*3600-30*60);
    ui->comboBoxUTCOffset->addItem("UTC-03:00",-3*3600);
    ui->comboBoxUTCOffset->addItem("UTC-02:00",-2*3600);
    ui->comboBoxUTCOffset->addItem("UTC-01:00",-1*3600);
    ui->comboBoxUTCOffset->addItem("UTC 00:00",0);
    ui->comboBoxUTCOffset->addItem("UTC+01:00",1*3600);
    ui->comboBoxUTCOffset->addItem("UTC+02:00",2*3600);
    ui->comboBoxUTCOffset->addItem("UTC+03:00",3*3600);
    ui->comboBoxUTCOffset->addItem("UTC+03:30",3*3600+30*60);
    ui->comboBoxUTCOffset->addItem("UTC+04:00",4*3600);
    ui->comboBoxUTCOffset->addItem("UTC+04:30",4*3600+30*60);
    ui->comboBoxUTCOffset->addItem("UTC+05:00",5*3600);
    ui->comboBoxUTCOffset->addItem("UTC+05:30",5*3600+30*60);
    ui->comboBoxUTCOffset->addItem("UTC+05:45",5*3600+45*60);
    ui->comboBoxUTCOffset->addItem("UTC+06:00",6*3600);
    ui->comboBoxUTCOffset->addItem("UTC+06:30",6*3600+30*60);
    ui->comboBoxUTCOffset->addItem("UTC+07:00",7*3600);
    ui->comboBoxUTCOffset->addItem("UTC+08:00",8*3600);
    ui->comboBoxUTCOffset->addItem("UTC+08:45",8*3600+45*60);
    ui->comboBoxUTCOffset->addItem("UTC+09:00",9*3600);
    ui->comboBoxUTCOffset->addItem("UTC+09:30",9*3600+30*60);
    ui->comboBoxUTCOffset->addItem("UTC+10:00",10*3600);
    ui->comboBoxUTCOffset->addItem("UTC+10:30",10*3600+30*60);
    ui->comboBoxUTCOffset->addItem("UTC+11:00",11*3600);
    ui->comboBoxUTCOffset->addItem("UTC+11:30",11*3600+30*60);
    ui->comboBoxUTCOffset->addItem("UTC+12:00",12*3600);
    ui->comboBoxUTCOffset->addItem("UTC+12:45",12*3600+45*60);
    ui->comboBoxUTCOffset->addItem("UTC+13:00",13*3600);
    ui->comboBoxUTCOffset->addItem("UTC+14:00",14*3600);
}

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SettingsDialog::assertSettingsVersion()
{
    DltSettingsManager *settings = DltSettingsManager::getInstance();

    int major = settings->value("startup/versionMajor").toInt();
    int minor = settings->value("startup/versionMinor").toInt();

    if(major == 0 && minor == 0)
        return; // The settings were empty already

    if(major < QString(PACKAGE_MAJOR_VERSION).toInt() ||
       minor < QString(PACKAGE_MINOR_VERSION).toInt())
    {
        QString msg;
        msg.append("Application version has changed. Settings might be incompatible.\n");
        msg.append("Would you like to remove all old settings and initialize new ones?\n");
        msg.append("Yes    - Reset settings to factory defaults.\n");
        msg.append("No     - Continue loading settings and risk crashing the application.\n");
        msg.append("Cancel - Exit the viewer now.\n");
        QMessageBox dlg("Warning", msg, QMessageBox::Warning,
                        QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        int btn = dlg.exec();
        if(btn == QMessageBox::Yes)
        {
            resetSettings();
        }
        else if(btn == QMessageBox::Cancel)
        {
            exit(-1);
        }
    }
}

void SettingsDialog::resetSettings()
{
    DltSettingsManager *settings = DltSettingsManager::getInstance();
    settings->clear();
    QString fn(settings->fileName());
    DltSettingsManager::close();
    QFile fh(fn);
    if(fh.exists())
    {
        if(!fh.open(QIODevice::ReadWrite))
            return; // Could be a registry key on windows
        fh.close();
        if(!fh.remove())
        {
            QMessageBox err("Error", "Could not remove the settings file", QMessageBox::Critical, QMessageBox::Ok, 0, 0);
            err.exec();
        }
    }
}

void SettingsDialog::writeDlg()
{
    /* Temp file */
    if(tempUseOwn == 1)
    {
        ui->radioButtonSelectTemp->setChecked(true);
    }
    else if(tempUseSystem == 1)
    {
        ui->radioButtonUseTemp->setChecked(true);
    }
    // ^Else, uses default set in .ui

    ui->lineEditSystemTemp->setText(QDir::tempPath());
    ui->lineEditOwnTemp->setText(tempOwnPath);
    ui->checkBoxCloseWithoutAsking->setChecked(tempCloseWithoutAsking == 1 ? true : false);
    ui->checkBoxSaveOnClear->setChecked(tempSaveOnClear == 1 ? true : false);

    /* startup */
    ui->checkBoxDefaultProjectFile->setCheckState(defaultProjectFile?Qt::Checked:Qt::Unchecked);
    ui->lineEditDefaultProjectFile->setText(defaultProjectFileName);
    ui->checkBoxDefaultLogFile->setCheckState(defaultLogFile?Qt::Checked:Qt::Unchecked);
    ui->lineEditDefaultLogFile->setText(defaultLogFileName);
    ui->checkBoxPluginsPath->setCheckState(pluginsPath?Qt::Checked:Qt::Unchecked);
    ui->lineEditPluginsPath->setText(pluginsPathName);
    ui->checkBoxDefaultFilterPath->setCheckState(defaultFilterPath?Qt::Checked:Qt::Unchecked);
    ui->lineEditDefaultFilterPath->setText(defaultFilterPathName);
    ui->checkBoxPluginsAutoload->setCheckState(pluginsAutoloadPath?Qt::Checked:Qt::Unchecked);
    ui->lineEditPluginsAutoload->setText(pluginsAutoloadPathName);
    ui->checkBoxFilterCache->setCheckState(filterCache?Qt::Checked:Qt::Unchecked);
    ui->spinBoxIndexCacheDays->setValue(filterCacheDays);
    ui->lineEditFilterCache->setText(filterCacheName);
    ui->checkBoxAutoConnect->setCheckState(autoConnect?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoScroll->setCheckState(autoScroll?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoMarkFatalError->setCheckState(autoMarkFatalError?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoMarkWarn->setCheckState(autoMarkWarn?Qt::Checked:Qt::Unchecked);

    /* table */
    ui->spinBoxFontSize->setValue(fontSize);

    /* Time settings */
    ui->groupBoxAutomaticTimeSettings->setChecked(automaticTimeSettings?Qt::Checked:Qt::Unchecked);
    if(ui->groupBoxAutomaticTimeSettings->isChecked()){
        ui->checkBoxDST->setEnabled(false);
        ui->comboBoxUTCOffset->setEnabled(false);
        ui->labelTimezone->setEnabled(false);
        ui->checkBoxAutomaticTimezone->setEnabled(false);
    }else{
        ui->checkBoxDST->setEnabled(true);
        ui->comboBoxUTCOffset->setEnabled(true);
        ui->labelTimezone->setEnabled(true);
        ui->checkBoxAutomaticTimezone->setEnabled(true);
    }
    ui->checkBoxAutomaticTimezone->setChecked(automaticTimezoneFromDlt?Qt::Checked:Qt::Unchecked);
    ui->checkBoxDST->setCheckState(dst?Qt::Checked:Qt::Unchecked);
    ui->comboBoxUTCOffset->setCurrentIndex(ui->comboBoxUTCOffset->findData(QVariant(utcOffset)));

    ui->checkBoxIndex->setCheckState(showIndex?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTime->setCheckState(showTime?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTimestamp->setCheckState(showTimestamp?Qt::Checked:Qt::Unchecked);
    ui->checkBoxCount->setCheckState(showCount?Qt::Checked:Qt::Unchecked);

    ui->checkBoxEcuid->setCheckState(showEcuId?Qt::Checked:Qt::Unchecked);

    ui->groupBoxAppId->setChecked(showApId?Qt::Checked:Qt::Unchecked);
    if(ui->groupBoxAppId->isChecked()){
        ui->radioButtonAppId->setEnabled(true);
        ui->radioButtonAppIdDesc->setEnabled(true);
    }else{
        ui->radioButtonAppId->setEnabled(false);
        ui->radioButtonAppIdDesc->setEnabled(false);
    }
    switch(showApIdDesc){
    case 0:
        ui->radioButtonAppId->setChecked(true);
        ui->radioButtonAppIdDesc->setChecked(false);
        break;
    case 1:
        ui->radioButtonAppId->setChecked(false);
        ui->radioButtonAppIdDesc->setChecked(true);
        break;
    default:
        ui->radioButtonConId->setChecked(true);
        ui->radioButtonConIdDesc->setChecked(false);
        break;
    }

    ui->groupBoxConId->setChecked(showCtId?Qt::Checked:Qt::Unchecked);
    if(ui->groupBoxConId->isChecked()){
        ui->radioButtonConId->setEnabled(true);
        ui->radioButtonConIdDesc->setEnabled(true);
    }else{
        ui->radioButtonConId->setEnabled(false);
        ui->radioButtonConIdDesc->setEnabled(false);
    }
    switch(showCtIdDesc){
    case 0:
        ui->radioButtonConId->setChecked(true);
        ui->radioButtonConIdDesc->setChecked(false);
        break;
    case 1:
        ui->radioButtonConId->setChecked(false);
        ui->radioButtonConIdDesc->setChecked(true);
        break;
    default:
        ui->radioButtonConId->setChecked(true);
        ui->radioButtonConIdDesc->setChecked(false);
        break;
    }

    ui->checkBoxType->setCheckState(showType?Qt::Checked:Qt::Unchecked);

    ui->checkBoxSubtype->setCheckState(showSubtype?Qt::Checked:Qt::Unchecked);
    ui->checkBoxMode->setCheckState(showMode?Qt::Checked:Qt::Unchecked);
    ui->checkBoxNoar->setCheckState(showNoar?Qt::Checked:Qt::Unchecked);
    ui->checkBoxPayload->setCheckState(showPayload?Qt::Checked:Qt::Unchecked);

    /* other */
    ui->checkBoxWriteControl->setCheckState(writeControl?Qt::Checked:Qt::Unchecked);
    ui->checkBoxUpdateContextLoadingFile->setCheckState(updateContextLoadingFile?Qt::Checked:Qt::Unchecked);
    ui->checkBoxUpdateContextUnregister->setCheckState(updateContextsUnregister?Qt::Checked:Qt::Unchecked);

    DltSettingsManager *settings = DltSettingsManager::getInstance();
    int refreshrate = settings->value("RefreshRate",DEFAULT_REFRESH_RATE).toInt();
    ui->spinBoxFrequency->setValue(refreshrate);

    bool startup_minimized = settings->value("StartUpMinimized",false).toBool();
    ui->checkBoxStartUpMinimized->setChecked(startup_minimized);


}

void SettingsDialog::readDlg()
{
    /* Temp file */
    tempUseSystem               = (ui->radioButtonUseTemp->isChecked() == true ? 1 : 0);
    tempSystemPath              = ui->lineEditSystemTemp->text();
    tempUseOwn                  = (ui->radioButtonSelectTemp->isChecked() == true ? 1 : 0);
    tempOwnPath                 = ui->lineEditOwnTemp->text();
    tempCloseWithoutAsking      = (ui->checkBoxCloseWithoutAsking->isChecked() == true ? 1 : 0);
    tempSaveOnClear             = (ui->checkBoxSaveOnClear->isChecked() == true ? 1 : 0);

    /* startup */
    defaultProjectFile = (ui->checkBoxDefaultProjectFile->checkState() == Qt::Checked);
    defaultProjectFileName = ui->lineEditDefaultProjectFile->text();
    defaultLogFile = (ui->checkBoxDefaultLogFile->checkState() == Qt::Checked);
    defaultLogFileName = ui->lineEditDefaultLogFile->text();
    pluginsPath = (ui->checkBoxPluginsPath->checkState() == Qt::Checked);
    pluginsPathName = ui->lineEditPluginsPath->text();
    defaultFilterPath = (ui->checkBoxDefaultFilterPath->checkState() == Qt::Checked);
    defaultFilterPathName = ui->lineEditDefaultFilterPath->text();
    pluginsAutoloadPath = (ui->checkBoxPluginsAutoload->checkState() == Qt::Checked);
    pluginsAutoloadPathName = ui->lineEditPluginsAutoload->text();
    filterCache = (ui->checkBoxFilterCache->checkState() == Qt::Checked);
    filterCacheDays = ui->spinBoxIndexCacheDays->value();
    filterCacheName = ui->lineEditFilterCache->text();
    autoConnect = (ui->checkBoxAutoConnect->checkState() == Qt::Checked);
    autoScroll = (ui->checkBoxAutoScroll->checkState() == Qt::Checked);
    autoMarkFatalError = (ui->checkBoxAutoMarkFatalError->checkState() == Qt::Checked);
    autoMarkWarn = (ui->checkBoxAutoMarkWarn->checkState() == Qt::Checked);

    /* table */
    fontSize = ui->spinBoxFontSize->value();

    /* Time settings */
    automaticTimeSettings = ( ui->groupBoxAutomaticTimeSettings->isChecked() == true ? 1:0);
    automaticTimezoneFromDlt = ( ui->checkBoxAutomaticTimezone->isChecked() == true ? 1:0);
    utcOffset = ui->comboBoxUTCOffset->itemData(ui->comboBoxUTCOffset->currentIndex()).toLongLong();
    dst =           ( ui->checkBoxDST->isChecked()== true ? 1:0);

    showIndex =     ( ui->checkBoxIndex->checkState() == Qt::Checked);
    showTime =      ( ui->checkBoxTime->checkState() == Qt::Checked);
    showTimestamp = ( ui->checkBoxTimestamp->checkState() == Qt::Checked);
    showCount =     ( ui->checkBoxCount->checkState() == Qt::Checked);

    showEcuId =     ( ui->checkBoxEcuid->checkState() == Qt::Checked);
    showApId =      ( ui->groupBoxAppId->isChecked() == true ? 1:0);
    showApIdDesc =  ( ui->radioButtonAppIdDesc->isChecked()== true ? 1:0);
    showCtId =      ( ui->groupBoxConId->isChecked() == true ? 1:0);
    showCtIdDesc =  ( ui->radioButtonConIdDesc->isChecked()== true ? 1:0);
    showType =      ( ui->checkBoxType->checkState() == Qt::Checked);

    showSubtype = ( ui->checkBoxSubtype->checkState() == Qt::Checked);
    showMode = ( ui->checkBoxMode->checkState() == Qt::Checked);
    showNoar = ( ui->checkBoxNoar->checkState() == Qt::Checked);
    showPayload = ( ui->checkBoxPayload->checkState() == Qt::Checked);

    /* other */
    writeControl = (ui->checkBoxWriteControl->checkState() == Qt::Checked);
    updateContextLoadingFile = (ui->checkBoxUpdateContextLoadingFile->checkState() == Qt::Checked);
    updateContextsUnregister = (ui->checkBoxUpdateContextUnregister->checkState() == Qt::Checked);

    DltSettingsManager *settings = DltSettingsManager::getInstance();
    int refreshrate = ui->spinBoxFrequency->value();
    int old_refreshrate = settings->value("RefreshRate",DEFAULT_REFRESH_RATE).toInt();
    if ( refreshrate != old_refreshrate && 0 != refreshrate )
        settings->setValue("RefreshRate",refreshrate);

    bool startup_minimized = ui->checkBoxStartUpMinimized->isChecked();
    settings->setValue("StartUpMinimized",startup_minimized);
}

void SettingsDialog::writeSettings(QMainWindow *mainwindow)
{
    DltSettingsManager *settings = DltSettingsManager::getInstance();

    settings->setValue("geometry", mainwindow->saveGeometry());
    settings->setValue("windowState", mainwindow->saveState());

    /* Temporary directory */
    settings->setValue("tempdir/tempUseSystem", tempUseSystem);
    settings->setValue("tempdir/tempSystemPath", tempSystemPath);
    settings->setValue("tempdir/tempUseOwn", tempUseOwn);
    settings->setValue("tempdir/tempOwnPath", tempOwnPath);
    settings->setValue("tempdir/tempCloseWithoutAsking", tempCloseWithoutAsking);
    settings->setValue("tempdir/tempSaveOnClear", tempSaveOnClear);

    /* startup */
    settings->setValue("startup/defaultProjectFile",defaultProjectFile);
    settings->setValue("startup/defaultProjectFileName",defaultProjectFileName);
    settings->setValue("startup/defaultLogFile",defaultLogFile);
    settings->setValue("startup/defaultLogFileName",defaultLogFileName);
    settings->setValue("startup/pluginsPath",pluginsPath);
    settings->setValue("startup/pluginsPathName",pluginsPathName);
    settings->setValue("startup/defaultFilterPath",defaultFilterPath);
    settings->setValue("startup/defaultFilterPathName",defaultFilterPathName);
    settings->setValue("startup/pluginsAutoloadPath",pluginsAutoloadPath);
    settings->setValue("startup/pluginsAutoloadPathName",pluginsAutoloadPathName);
    settings->setValue("startup/filterCache",filterCache);
    settings->setValue("startup/filterCacheDays",filterCacheDays);
    settings->setValue("startup/filterCacheName",filterCacheName);
    settings->setValue("startup/autoConnect",autoConnect);
    settings->setValue("startup/autoScroll",autoScroll);
    settings->setValue("startup/autoMarkFatalError",autoMarkFatalError);
    settings->setValue("startup/autoMarkWarn",autoMarkWarn);

    /* table */
    settings->setValue("startup/fontSize",fontSize);
    settings->setValue("startup/automaticTimeSettings",automaticTimeSettings);
    settings->setValue("startup/automaticTimezoneFromDlt",automaticTimezoneFromDlt);
    settings->setValue("startup/utcOffset",utcOffset);
    settings->setValue("startup/dst",dst);
    settings->setValue("startup/showIndex",showIndex);
    settings->setValue("startup/showTime",showTime);
    settings->setValue("startup/showTimestamp",showTimestamp);
    settings->setValue("startup/showCount",showCount);

    settings->setValue("startup/showEcuId",showEcuId);
    settings->setValue("startup/showApId",showApId);
    settings->setValue("startup/showApIdDesc",showApIdDesc);
    settings->setValue("startup/showCtId",showCtId);
    settings->setValue("startup/showCtIdDesc",showCtIdDesc);
    settings->setValue("startup/showType",showType);

    settings->setValue("startup/showSubtype",showSubtype);
    settings->setValue("startup/showMode",showMode);
    settings->setValue("startup/showNoar",showNoar);
    settings->setValue("startup/showPayload",showPayload);

    /* other */
    settings->setValue("startup/writeControl",writeControl);
    settings->setValue("startup/updateContextLoadingFile",updateContextLoadingFile);
    settings->setValue("startup/updateContextsUnregister",updateContextsUnregister);

    /* For settings integrity validation */
    settings->setValue("startup/versionMajor", QString(PACKAGE_MAJOR_VERSION).toInt());
    settings->setValue("startup/versionMinor", QString(PACKAGE_MINOR_VERSION).toInt());
    settings->setValue("startup/versionPatch", QString(PACKAGE_PATCH_LEVEL).toInt());
}

void SettingsDialog::readSettings()
{
    DltSettingsManager *settings = DltSettingsManager::getInstance();

    /* Temp file */
    tempUseSystem               = settings->value("tempdir/tempUseSystem", 1).toInt();
    tempSystemPath              = QDir::tempPath();
    tempUseOwn                  = settings->value("tempdir/tempUseOwn", 0).toInt();
    tempOwnPath                 = settings->value("tempdir/tempOwnPath", QString("")).toString();
    tempCloseWithoutAsking      = settings->value("tempdir/tempCloseWithoutAsking", 0).toInt();
    tempSaveOnClear             = settings->value("tempdir/tempSaveOnClear", 1).toInt();

    /* startup */
    defaultProjectFile = settings->value("startup/defaultProjectFile",0).toInt();
    defaultProjectFileName = settings->value("startup/defaultProjectFileName",QString("")).toString();
    defaultLogFile = settings->value("startup/defaultLogFile",0).toInt();
    defaultLogFileName = settings->value("startup/defaultLogFileName",QString("")).toString();
    pluginsPath = settings->value("startup/pluginsPath",0).toInt();
    pluginsPathName = settings->value("startup/pluginsPathName",QString("")).toString();
    defaultFilterPath = settings->value("startup/defaultFilterPath",1).toInt();
    defaultFilterPathName = settings->value("startup/defaultFilterPathName",QCoreApplication::applicationDirPath()+"/filters").toString();
    pluginsAutoloadPath = settings->value("startup/pluginsAutoloadPath",0).toInt();
    pluginsAutoloadPathName = settings->value("startup/pluginsAutoloadPathName",QString("")).toString();
    filterCache = settings->value("startup/filterCache",1).toInt();
    filterCacheDays = settings->value("startup/filterCacheDays",7).toInt();
    filterCacheName = settings->value("startup/filterCacheName",QCoreApplication::applicationDirPath()+"/cache").toString();
    autoConnect = settings->value("startup/autoConnect",0).toInt();
    autoScroll = settings->value("startup/autoScroll",1).toInt();
    autoMarkFatalError = settings->value("startup/autoMarkFatalError",0).toInt();
    autoMarkWarn = settings->value("startup/autoMarkWarn",0).toInt();

    /* table */
    fontSize = settings->value("startup/fontSize",8).toInt();
    automaticTimeSettings = settings->value("startup/automaticTimeSettings",1).toInt();
    automaticTimezoneFromDlt = settings->value("startup/automaticTimezoneFromDlt",1).toInt();
    utcOffset = settings->value("startup/utcOffset",QVariant((qlonglong)timezone*-1)).toLongLong();
    dst = settings->value("startup/dst",daylight == 0 ? 0 : 1).toInt();
    showIndex = settings->value("startup/showIndex",1).toInt();
    showTime = settings->value("startup/showTime",1).toInt();
    showTimestamp = settings->value("startup/showTimestamp",1).toInt();
    showCount = settings->value("startup/showCount",1).toInt();

    showEcuId = settings->value("startup/showEcuId",1).toInt();
    showApId = settings->value("startup/showApId",1).toInt();
    showApIdDesc = settings->value("startup/showApIdDesc",0).toInt();
    showCtId = settings->value("startup/showCtId",1).toInt();
    showCtIdDesc = settings->value("startup/showCtIdDesc",0).toInt();
    showType = settings->value("startup/showType",1).toInt();

    showSubtype = settings->value("startup/showSubtype",1).toInt();
    showMode = settings->value("startup/showMode",1).toInt();
    showNoar = settings->value("startup/showNoar",1).toInt();
    showPayload = settings->value("startup/showPayload",1).toInt();

    /* other */
    writeControl = settings->value("startup/writeControl",1).toInt();
    updateContextLoadingFile = settings->value("startup/updateContextLoadingFile",1).toInt();
    updateContextsUnregister = settings->value("startup/updateContextsUnregister",0).toInt();
}



QStringList SettingsDialog::getRecentFiles(){
    return DltSettingsManager::getInstance()->value("other/recentFileList").toStringList();
}
QStringList SettingsDialog::getRecentProjects(){
    return DltSettingsManager::getInstance()->value("other/recentProjectList").toStringList();
}
QStringList SettingsDialog::getRecentFilters(){
    return DltSettingsManager::getInstance()->value("other/recentFiltersList").toStringList();
}
QString SettingsDialog::getWorkingDirectory(){
    return DltSettingsManager::getInstance()->value("work/workingDirectory",QDir::currentPath()).toString();
}

void SettingsDialog::on_toolButtonDefaultLogFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Log file"), workingDirectory, tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    ui->lineEditDefaultLogFile->setText(fileName);

}

void SettingsDialog::on_toolButtonDefaultProjectFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Project file"), workingDirectory, tr("DLT Project Files (*.dlp);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    ui->lineEditDefaultProjectFile->setText(fileName);
}

void SettingsDialog::on_tooButtonPluginsPath_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Plugins directory"), workingDirectory+"/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    ui->lineEditPluginsPath->setText(fileName);

    QMessageBox::warning(0, QString("DLT Viewer"),
                         QString("Plugins will only be reloaded after restart of DLT Viewer!"));

}


void SettingsDialog::on_toolButtonDefaultFilterPath_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Default Filter directory"), workingDirectory+"/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    ui->lineEditDefaultFilterPath->setText(fileName);

    QMessageBox::warning(0, QString("DLT Viewer"),
                         QString("Default filters will only be reloaded when triggered in the menu!"));
}

void SettingsDialog::on_toolButtonTempPath_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Temporary directory"), workingDirectory+"/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    tempOwnPath = QFileInfo(fileName).absolutePath();

    ui->lineEditOwnTemp->setText(fileName);
}

void SettingsDialog::on_toolButtonPluginsAutoload_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Plugins configuration directory"), workingDirectory+"/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    pluginsAutoloadPathName = QFileInfo(fileName).absolutePath();

    ui->lineEditPluginsAutoload->setText(fileName);
}


void SettingsDialog::on_toolButtonFilterCache_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Filter Cache directory"), workingDirectory+"/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    filterCacheName = QFileInfo(fileName).absolutePath();

    ui->lineEditFilterCache->setText(fileName);
}

void SettingsDialog::on_groupBoxConId_clicked(bool checked)
{
    if(checked){
        ui->radioButtonConId->setEnabled(true);
        ui->radioButtonConIdDesc->setEnabled(true);
    }else{
        ui->radioButtonConId->setEnabled(false);
        ui->radioButtonConIdDesc->setEnabled(false);
    }
}

void SettingsDialog::on_groupBoxAppId_clicked(bool checked)
{
    if(checked){
        ui->radioButtonAppId->setEnabled(true);
        ui->radioButtonAppIdDesc->setEnabled(true);
    }else{
        ui->radioButtonAppId->setEnabled(false);
        ui->radioButtonAppIdDesc->setEnabled(false);
    }
}

void SettingsDialog::on_groupBoxAutomaticTimeSettings_clicked(bool checked)
{
    if(checked){
        ui->checkBoxDST->setEnabled(false);
        ui->comboBoxUTCOffset->setEnabled(false);
        ui->labelTimezone->setEnabled(false);
        ui->checkBoxAutomaticTimezone->setEnabled(false);
    }else{
        ui->checkBoxDST->setEnabled(true);
        ui->comboBoxUTCOffset->setEnabled(true);
        ui->labelTimezone->setEnabled(true);
        ui->checkBoxAutomaticTimezone->setEnabled(true);
    }
}


void SettingsDialog::on_pushButtonClearIndexCache_clicked()
{
    QString path = ui->lineEditFilterCache->text();
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.dix");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }
}

void SettingsDialog::clearIndexCacheAfterDays()
{
    // calculate comparison date
    QDateTime comparisonDate(QDateTime::currentDateTime());
    comparisonDate = comparisonDate.addSecs((quint64)filterCacheDays*-1*60*60*24);

    // check if index cache is enabled
    if(!filterCache)
        return;

    // go through each file and check modification date of file
    QString path = filterCacheName;
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.dix");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        QFileInfo info(path+"/"+dirFile);
        QDateTime fileDate = info.lastRead();

        //qDebug() << fileDate.toString("dd.MM.yyyy hh:mm:ss.zzz") << comparisonDate.toString("dd.MM.yyyy hh:mm:ss.zzz");

        if(fileDate < comparisonDate)
            dir.remove(dirFile);
    }
}

