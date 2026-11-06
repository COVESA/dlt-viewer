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
 * \file plugindialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QFileDialog>

#include "plugindialog.h"
#include "ui_plugindialog.h"

PluginDialog::PluginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginDialog),
    multiSelectEnabled(false) {
    ui->setupUi(this);
}

PluginDialog::~PluginDialog() {
    delete ui;
}

void PluginDialog::setName(QString name) {
    ui->lineEditName->setText(name);

}

void PluginDialog::setPluginVersion(QString version){
    ui->lineEditPluginVersion->setText(version);
}

void PluginDialog::setPluginInterfaceVersion(QString version){
    ui->lineEditPluginInterfaceVersion->setText(version);
}

void PluginDialog::setMode(int mode) {
    ui->comboBoxMode->setCurrentIndex(mode);
}

void PluginDialog::removeMode(int mode) {
    ui->comboBoxMode->removeItem(mode);
}

void PluginDialog::setType(int type) {
    ui->comboBoxType->setCurrentIndex(type);
}

void PluginDialog::setFilename(QString filename) {
    ui->lineEditFilename->setText(filename);
}

void PluginDialog::setMultiSelectionEnabled(bool enabled) {
    multiSelectEnabled = enabled;
}

int PluginDialog::getMode() {
    return ui->comboBoxMode->currentIndex();
}

int PluginDialog::getType() {
    return ui->comboBoxType->currentIndex();
}

QString PluginDialog::getFilename() {
    return ui->lineEditFilename->text();
}

void PluginDialog::on_toolButton_clicked() {
    QString name = ui->lineEditName->text();
    QString fileName;
    QString lastPickedPath;

    if(ui->comboBoxType->currentIndex()==0)
    {
        // filename
        if(multiSelectEnabled)
        {
            QStringList fileNames = QFileDialog::getOpenFileNames(this,
                QString("Open ")+name+QString(" configuration file"),
                workingDirectory.getPluginDirectory(name),
                tr("Plugin configuration (*.*)"));
            if(fileNames.isEmpty())
                return;
            fileName = fileNames.join('|');
            lastPickedPath = fileNames.first();
        }
        else
        {
            fileName = QFileDialog::getOpenFileName(this,
                QString("Open ")+name+QString(" configuration file"),
                workingDirectory.getPluginDirectory(name),
                tr("Plugin configuration (*.*)"));
            lastPickedPath = fileName;
        }
    }
    else
    {
        // directory
        QString dirName = QFileDialog::getExistingDirectory(this,
                                                     QString("Open ")+name+QString(" configuration file"),
                                                     workingDirectory.getPluginDirectory(name),
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(dirName.isEmpty())
            return;
        lastPickedPath = dirName;

        if(multiSelectEnabled)
        {
            // Native dialogs only support picking one directory at a time, so
            // accumulate directories across repeated clicks of the browse
            // button instead, joined by '|' (same separator used for multiple
            // files above).
            QStringList dirs = ui->lineEditFilename->text().split('|', Qt::SkipEmptyParts);
            if(!dirs.contains(dirName))
                dirs.append(dirName);
            fileName = dirs.join('|');
        }
        else
        {
            fileName = dirName;
        }
    }

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory.setPluginDirectory(name, QFileInfo(lastPickedPath).absolutePath());

    ui->lineEditFilename->setText(fileName);
}
