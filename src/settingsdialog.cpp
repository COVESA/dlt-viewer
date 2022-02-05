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
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QStandardPaths>
#include <QFontDialog>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "version.h"
#include "dltuiutils.h"


#if (WIN32)
 #define TZSET _tzset()
 #define TIMEZONE _timezone
 #define DAYLIGHT _daylight
#else
 #define TZSET tzset()
 #define TIMEZONE timezone
 #define DAYLIGHT daylight
#endif


SettingsDialog::SettingsDialog(QDltFile *_qFile, QWidget *parent):

    QDialog(parent), qFile(_qFile),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    //See man pages - tzset(3)
    //It is used to access DST and other timezone related information
    //and sets all timezone global variables
     TZSET;
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

    ui->comboBox_MessageIdFormat->addItem("%010u");
    ui->comboBox_MessageIdFormat->addItem("%u");
    ui->comboBox_MessageIdFormat->addItem("0x%08X");
    ui->comboBox_MessageIdFormat->addItem("0x%08x");
    ui->comboBox_MessageIdFormat->addItem("%08xh");

    QDltSettingsManager *settings = QDltSettingsManager::getInstance();
    settings->fmaxFileSizeMB = 0.0;
    settings->appendDateTime = 0;
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
   QDltSettingsManager *settings = QDltSettingsManager::getInstance();

   int major = settings->value("startup/versionMajor").toInt();
   int minor = settings->value("startup/versionMinor").toInt();

   if(major == 0 && minor == 0)
    {
        return; // The settings were empty already
    }

   if(major > PACKAGE_MAJOR_VERSION || minor > PACKAGE_MINOR_VERSION)
    {
        QString msg;
        msg.append("The application version has changed ! The settings file config.ini might be incompatible.\n");
        msg.append("Would you like to remove all old settings and initialize new ones?\n");
        msg.append("Yes    - Reset settings to factory defaults.\n");
        msg.append("No     - Continue loading settings and risk crashing the application.\n");
        msg.append("Cancel - Exit the viewer now.\n");
        QMessageBox dlg("Warning", msg, QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
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
    QDltSettingsManager *settings = QDltSettingsManager::getInstance();
    settings->clear();
    QString fn(settings->fileName());
    QFile fh(fn);
    if(true == fh.exists())
    {
        if(false == fh.open(QIODevice::ReadWrite))
            return; // Could be a registry key on windows
        fh.close();
        if(false == fh.remove())
        {
            QMessageBox err("Error", "Could not remove the settings file", QMessageBox::Critical, QMessageBox::Ok, 0, 0);
            err.exec();
        }
        else
        {
          qDebug() << "Deleted settings file" << fn << fh.exists();
          readSettings();
        }
    }
}

void SettingsDialog::writeDlg()
{
    QDltSettingsManager *settings = QDltSettingsManager::getInstance();

    /* Temp file */
    if(settings->tempUseOwn == 1)
    {
        ui->radioButtonSelectTemp->setChecked(true);
    }
    else if(settings->tempUseSystem == 1)
    {
        ui->radioButtonUseTemp->setChecked(true);
    }
    // ^Else, uses default set in .ui

    ui->lineEditSystemTemp->setText(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    ui->lineEditOwnTemp->setText(settings->tempOwnPath);
    ui->checkBoxCloseWithoutAsking->setChecked(settings->tempCloseWithoutAsking == 1 ? true : false);
    ui->checkBoxSaveOnClear->setChecked(settings->tempSaveOnClear == 1 ? true : false);

    /* startup */
    ui->checkBoxDefaultProjectFile->setCheckState(settings->defaultProjectFile?Qt::Checked:Qt::Unchecked);
    ui->lineEditDefaultProjectFile->setText(settings->defaultProjectFileName);
    ui->checkBoxDefaultLogFile->setCheckState(settings->defaultLogFile?Qt::Checked:Qt::Unchecked);
    ui->lineEditDefaultLogFile->setText(settings->defaultLogFileName);
    ui->checkBoxPluginsPath->setCheckState(settings->pluginsPath?Qt::Checked:Qt::Unchecked);
    ui->lineEditPluginsPath->setText(settings->pluginsPathName);
    ui->checkBoxDefaultFilterPath->setCheckState(settings->defaultFilterPath?Qt::Checked:Qt::Unchecked);
    ui->lineEditDefaultFilterPath->setText(settings->defaultFilterPathName);
    ui->checkBoxPluginsAutoload->setCheckState(settings->pluginsAutoloadPath?Qt::Checked:Qt::Unchecked);
    ui->lineEditPluginsAutoload->setText(settings->pluginsAutoloadPathName);
    ui->checkBoxFilterCache->setCheckState(settings->filterCache?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoConnect->setCheckState(settings->autoConnect?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoScroll->setCheckState(settings->autoScroll?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoMarkFatalError->setCheckState(settings->autoMarkFatalError?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoMarkWarn->setCheckState(settings->autoMarkWarn?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoMarkMarker->setCheckState(settings->autoMarkMarker?Qt::Checked:Qt::Unchecked);
    ui->checkBoxLoggingOnlyMode->setCheckState(settings->loggingOnlyMode?Qt::Checked:Qt::Unchecked);
    ui->groupBoxMaxFileSizeMB->setChecked(settings->splitlogfile?Qt::Checked:Qt::Unchecked);
    ui->lineEditMaxFileSizeMB->setText(QString("%1").arg(settings->fmaxFileSizeMB));
    ui->checkBoxAppendDateTime->setCheckState(settings->appendDateTime?Qt::Checked:Qt::Unchecked);

    /* table */
    ui->spinBoxSectionSize->setValue(settings->sectionSize);
    font.fromString(settings->fontName);
    ui->labelFont->setText(font.toString());

    /* Time settings */
    ui->groupBoxAutomaticTimeSettings->setChecked(settings->automaticTimeSettings);
    if(ui->groupBoxAutomaticTimeSettings->isChecked())
    {
        ui->checkBoxDST->setEnabled(false);
        ui->comboBoxUTCOffset->setEnabled(false);
        ui->labelTimezone->setEnabled(false);
        ui->checkBoxAutomaticTimezone->setEnabled(false);
    }
    else
    {
        ui->checkBoxDST->setEnabled(true);
        ui->comboBoxUTCOffset->setEnabled(true);
        ui->labelTimezone->setEnabled(true);
        ui->checkBoxAutomaticTimezone->setEnabled(true);
    }
    ui->checkBoxAutomaticTimezone->setChecked(settings->automaticTimezoneFromDlt);

    ui->checkBoxDST->setCheckState(settings->dst?Qt::Checked:Qt::Unchecked);
    ui->comboBoxUTCOffset->setCurrentIndex(ui->comboBoxUTCOffset->findData(QVariant(settings->utcOffset)));

    ui->checkBoxIndex->setCheckState(settings->showIndex?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTime->setCheckState(settings->showTime?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTimestamp->setCheckState(settings->showTimestamp?Qt::Checked:Qt::Unchecked);
    ui->checkBoxCount->setCheckState(settings->showCount?Qt::Checked:Qt::Unchecked);

    ui->checkBoxEcuid->setCheckState(settings->showEcuId?Qt::Checked:Qt::Unchecked);

    ui->groupBoxAppId->setChecked(settings->showApId);
    if(ui->groupBoxAppId->isChecked())
    {
        ui->radioButtonAppId->setEnabled(true);
        ui->radioButtonAppIdDesc->setEnabled(true);
    }
    else
    {
        ui->radioButtonAppId->setEnabled(false);
        ui->radioButtonAppIdDesc->setEnabled(false);
    }
    switch(settings->showApIdDesc)
    {
    case 0:
        ui->radioButtonAppId->setChecked(true);
        ui->radioButtonAppIdDesc->setChecked(false);
        break;
    case 1:
        ui->radioButtonAppId->setChecked(false);
        ui->radioButtonAppIdDesc->setChecked(true);
        break;
    default:
        ui->radioButtonAppId->setChecked(true);
        ui->radioButtonAppIdDesc->setChecked(false);
        break;
    }

    ui->groupBoxConId->setChecked(settings->showCtId);
    if(ui->groupBoxConId->isChecked())
    {
        ui->radioButtonConId->setEnabled(true);
        ui->radioButtonConIdDesc->setEnabled(true);
    }
    else
    {
        ui->radioButtonConId->setEnabled(false);
        ui->radioButtonConIdDesc->setEnabled(false);
    }
    switch(settings->showCtIdDesc)
    {
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

    ui->groupBoxSessionId->setChecked(settings->showSessionId);
    if(ui->groupBoxSessionId->isChecked())
    {
        ui->radioButtonSessionId->setEnabled(true);
        ui->radioButtonSessionName->setEnabled(true);
    }
    else
    {
        ui->radioButtonSessionId->setEnabled(false);
        ui->radioButtonSessionName->setEnabled(false);
    }
    switch(settings->showSessionName)
    {
    case 0:
        ui->radioButtonSessionId->setChecked(true);
        ui->radioButtonSessionName->setChecked(false);
        break;
    case 1:
        ui->radioButtonSessionId->setChecked(false);
        ui->radioButtonSessionName->setChecked(true);
        break;
    default:
        ui->radioButtonSessionId->setChecked(true);
        ui->radioButtonSessionName->setChecked(false);
        break;
    }

    ui->checkBoxType->setCheckState(settings->showType?Qt::Checked:Qt::Unchecked);

    ui->checkBoxSubtype->setCheckState(settings->showSubtype?Qt::Checked:Qt::Unchecked);
    ui->checkBoxMode->setCheckState(settings->showMode?Qt::Checked:Qt::Unchecked);
    ui->checkBoxNoar->setCheckState(settings->showNoar?Qt::Checked:Qt::Unchecked);
    ui->checkBoxPayload->setCheckState(settings->showPayload?Qt::Checked:Qt::Unchecked);
    ui->groupBoxMessageId->setChecked(settings->showMsgId?Qt::Checked:Qt::Unchecked);
    ui->spinBox_showArguments->setValue(settings->showArguments);

    /* other */
    ui->checkBoxWriteControl->setCheckState(settings->writeControl?Qt::Checked:Qt::Unchecked);
    ui->checkBoxUpdateContextLoadingFile->setCheckState(settings->updateContextLoadingFile?Qt::Checked:Qt::Unchecked);
    ui->checkBoxUpdateContextUnregister->setCheckState(settings->updateContextsUnregister?Qt::Checked:Qt::Unchecked);

    ui->spinBoxFrequency->setValue(settings->RefreshRate);
    ui->checkBoxStartUpMinimized->setChecked(settings->StartupMinimized);
    ui->comboBox_MessageIdFormat->setCurrentText(settings->msgIdFormat);
}

void SettingsDialog::readDlg()
{
    QDltSettingsManager *settings = QDltSettingsManager::getInstance();

    /* Temp file */
    settings->tempUseSystem               = (ui->radioButtonUseTemp->isChecked() == true ? 1 : 0);
    settings->tempSystemPath              = ui->lineEditSystemTemp->text();
    settings->tempUseOwn                  = (ui->radioButtonSelectTemp->isChecked() == true ? 1 : 0);
    settings->tempOwnPath                 = ui->lineEditOwnTemp->text();
    settings->tempCloseWithoutAsking      = (ui->checkBoxCloseWithoutAsking->isChecked() == true ? 1 : 0);
    settings->tempSaveOnClear             = (ui->checkBoxSaveOnClear->isChecked() == true ? 1 : 0);

    /* startup */
    settings->defaultProjectFile = (ui->checkBoxDefaultProjectFile->checkState() == Qt::Checked);
    settings->defaultProjectFileName = ui->lineEditDefaultProjectFile->text();
    settings->defaultLogFile = (ui->checkBoxDefaultLogFile->checkState() == Qt::Checked);
    settings->defaultLogFileName = ui->lineEditDefaultLogFile->text();
    settings->pluginsPath = (ui->checkBoxPluginsPath->checkState() == Qt::Checked);
    settings->pluginsPathName = ui->lineEditPluginsPath->text();
    settings->defaultFilterPath = (ui->checkBoxDefaultFilterPath->checkState() == Qt::Checked);
    settings->defaultFilterPathName = ui->lineEditDefaultFilterPath->text();
    settings->pluginsAutoloadPath = (ui->checkBoxPluginsAutoload->checkState() == Qt::Checked);
    settings->pluginsAutoloadPathName = ui->lineEditPluginsAutoload->text();
    settings->filterCache = (ui->checkBoxFilterCache->checkState() == Qt::Checked);
    settings->autoConnect = (ui->checkBoxAutoConnect->checkState() == Qt::Checked);
    settings->autoScroll = (ui->checkBoxAutoScroll->checkState() == Qt::Checked);
    settings->autoMarkFatalError = (ui->checkBoxAutoMarkFatalError->checkState() == Qt::Checked);
    settings->autoMarkWarn = (ui->checkBoxAutoMarkWarn->checkState() == Qt::Checked);
    settings->autoMarkMarker = (ui->checkBoxAutoMarkMarker->checkState() == Qt::Checked);
    settings->loggingOnlyMode = (ui->checkBoxLoggingOnlyMode->checkState() == Qt::Checked);
    settings->splitlogfile = ui->groupBoxMaxFileSizeMB->isChecked();
    if(settings->splitlogfile != 0)
     {
        settings->fmaxFileSizeMB = ui->lineEditMaxFileSizeMB->text().toFloat();
        if (settings->fmaxFileSizeMB < 0.01)
        {
          settings->fmaxFileSizeMB = (float) 0.01;
          qDebug() <<  "Caution: minimum split file size limited to 0.01 Mb !";
          //QMessageBox::warning(0, QString("DLT Viewer"), QString("Minimum value limited to 0.01 Mb !"));
        }
     }

    settings->appendDateTime = (ui->checkBoxAppendDateTime->checkState() == Qt::Checked);

    /* table */
    settings->sectionSize = ui->spinBoxSectionSize->value();
    settings->fontName = font.toString();

    /* Time settings */
    settings->automaticTimeSettings = ( ui->groupBoxAutomaticTimeSettings->isChecked() == true ? 1:0);
    settings->automaticTimezoneFromDlt = ( ui->checkBoxAutomaticTimezone->isChecked() == true ? 1:0);
    settings->utcOffset = ui->comboBoxUTCOffset->itemData(ui->comboBoxUTCOffset->currentIndex()).toLongLong();
    settings->dst =           ( ui->checkBoxDST->isChecked()== true ? 1:0);

    settings->showIndex =     ( ui->checkBoxIndex->checkState() == Qt::Checked);
    settings->showTime =      ( ui->checkBoxTime->checkState() == Qt::Checked);
    settings->showTimestamp = ( ui->checkBoxTimestamp->checkState() == Qt::Checked);
    settings->showCount =     ( ui->checkBoxCount->checkState() == Qt::Checked);

    settings->showEcuId =     ( ui->checkBoxEcuid->checkState() == Qt::Checked);
    settings->showApId =      ( ui->groupBoxAppId->isChecked() == true ? 1:0);
    settings->showApIdDesc =  ( ui->radioButtonAppIdDesc->isChecked()== true ? 1:0);
    settings->showCtId =      ( ui->groupBoxConId->isChecked() == true ? 1:0);
    settings->showCtIdDesc =  ( ui->radioButtonConIdDesc->isChecked()== true ? 1:0);
    settings->showSessionId =   ( ui->groupBoxSessionId->isChecked() == true ? 1:0);
    settings->showSessionName = ( ui->radioButtonSessionName->isChecked()== true ? 1:0);
    settings->showType =      ( ui->checkBoxType->checkState() == Qt::Checked);

    settings->showSubtype = ( ui->checkBoxSubtype->checkState() == Qt::Checked);
    settings->showMode = ( ui->checkBoxMode->checkState() == Qt::Checked);
    settings->showNoar = ( ui->checkBoxNoar->checkState() == Qt::Checked);
    settings->showPayload = ( ui->checkBoxPayload->checkState() == Qt::Checked);
    settings->showArguments = (ui->spinBox_showArguments->value());
    settings->showMsgId     = (ui->groupBoxMessageId->isChecked()==true?1:0);

    /* other */
    settings->writeControl = (ui->checkBoxWriteControl->checkState() == Qt::Checked);
    settings->updateContextLoadingFile = (ui->checkBoxUpdateContextLoadingFile->checkState() == Qt::Checked);
    settings->updateContextsUnregister = (ui->checkBoxUpdateContextUnregister->checkState() == Qt::Checked);

    settings->RefreshRate = ui->spinBoxFrequency->value();
    settings->StartupMinimized = ui->checkBoxStartUpMinimized->isChecked();
    settings->msgIdFormat=ui->comboBox_MessageIdFormat->currentText();
}

void SettingsDialog::writeSettings(QMainWindow *mainwindow)
{
    QDltSettingsManager *settings = QDltSettingsManager::getInstance();

    settings->geometry =  mainwindow->saveGeometry();
    settings->windowState = mainwindow->saveState();

    settings->writeSettings();
}

/* read the settings from config.ini */
void SettingsDialog::readSettings()
{
    QDltSettingsManager *settings = QDltSettingsManager::getInstance();

    settings->readSettings();

    QPalette palette = ui->labelSelectedMarkerColor->palette();
    palette.setColor(QPalette::Active,this->backgroundRole(),settings->markercolor);
    ui->labelSelectedMarkerColor->setPalette(palette);

}



QStringList SettingsDialog::getRecentFiles()
{
    return QDltSettingsManager::getInstance()->value("other/recentFileList").toStringList();
}
QStringList SettingsDialog::getRecentProjects()
{
    return QDltSettingsManager::getInstance()->value("other/recentProjectList").toStringList();
}
QStringList SettingsDialog::getRecentFilters(){
    return QDltSettingsManager::getInstance()->value("other/recentFiltersList").toStringList();
}
QString SettingsDialog::getWorkingDirectory()
{
    return QDltSettingsManager::getInstance()->value("work/workingDirectory",QDir::currentPath()).toString();
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
    emit(FilterPathChanged());
    //QMessageBox::warning(0, QString("DLT Viewer"), QString("Default filters will be reloaded at next program start !"));
}

void SettingsDialog::on_toolButtonTempPath_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Temporary directory"), workingDirectory+"/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    QDltSettingsManager *settings = QDltSettingsManager::getInstance();
    settings->tempOwnPath = QFileInfo(fileName).absolutePath();

    ui->lineEditOwnTemp->setText(fileName);
}

void SettingsDialog::on_toolButtonPluginsAutoload_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Plugins configuration directory"), workingDirectory+"/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    QDltSettingsManager *settings = QDltSettingsManager::getInstance();
    settings->pluginsAutoloadPathName = QFileInfo(fileName).absolutePath();

    ui->lineEditPluginsAutoload->setText(fileName);
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

void SettingsDialog::on_groupBoxSessionId_clicked(bool checked)
{
    if(checked){
        ui->radioButtonSessionId->setEnabled(true);
        ui->radioButtonSessionName->setEnabled(true);
    }else{
        ui->radioButtonSessionId->setEnabled(false);
        ui->radioButtonSessionName->setEnabled(false);
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
    if(checked)
    {
        ui->checkBoxDST->setEnabled(false);
        ui->comboBoxUTCOffset->setEnabled(false);
        ui->labelTimezone->setEnabled(false);
        ui->checkBoxAutomaticTimezone->setEnabled(false);
    }
    else
    {
        ui->checkBoxDST->setEnabled(true);
        ui->comboBoxUTCOffset->setEnabled(true);
        ui->labelTimezone->setEnabled(true);
        ui->checkBoxAutomaticTimezone->setEnabled(true);
    }
}

void SettingsDialog::on_checkBoxPluginsAutoload_stateChanged(int activated)
{
   if ( activated != 0)
    {
       emit(PluginsAutoloadChanged());
    }

}

void SettingsDialog::on_pushButtonMarkerColor_clicked()
{
    QColor selectedcolor = QColorDialog::getColor( ui->labelSelectedMarkerColor->palette().background().color().name() );
    QPalette palette = ui->labelSelectedMarkerColor->palette();
    palette.setColor(QPalette::Active,this->backgroundRole(),selectedcolor);
    palette.setColor(QPalette::Inactive,this->backgroundRole(),QColor(255,255,255,255));
    palette.setColor(QPalette::Foreground,DltUiUtils::optimalTextColor(selectedcolor));
    ui->labelSelectedMarkerColor->setPalette(palette);

    if(selectedcolor.isValid())
    {
        QDltSettingsManager *settings = QDltSettingsManager::getInstance();
        settings->markercolor =  selectedcolor;
    }
}

void SettingsDialog::on_pushButtonSelectFont_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(
                    &ok, this->font, this);
    if (ok) {
        // the user clicked OK and font is set to the font the user selected
        this->font = font;
        ui->labelFont->setText(font.toString());
    } else {
        // the user canceled the dialog; font is set to the initial
    }
}

void SettingsDialog::on_groupBoxArguments_clicked(bool checked)
{
    if (checked)
    {
      if (ui->spinBox_showArguments->value()<=0) ui->spinBox_showArguments->setValue(5); //set to default
    }
    else
    {
      ui->spinBox_showArguments->setValue(0);
    }
}

void SettingsDialog::on_spinBox_showArguments_valueChanged(int i)
{
 if (i<=0)
 {
  ui->groupBoxArguments->setChecked(false);
 }
 else
 {
     if (!ui->groupBoxArguments->isChecked()) ui->groupBoxArguments->setChecked(true);
 }
}
