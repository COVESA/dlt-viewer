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
 * \file form.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "form.h"
#include "ui_form.h"
#include "dummycontrolplugin.h"

#include <qfiledialog.h>

using namespace DummyControl;

Form::Form(DummyControlPlugin *_plugin,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    plugin = _plugin;
}

Form::~Form()
{
    delete ui;
}

void Form::setConnections(QStringList list)
{
    ui->comboBoxConnections->clear();
    ui->comboBoxConnections->insertItems(0,list);
}

void Form::autoscrollStateChanged(bool enabled)
{
    ui->checkBoxAutoscroll->setChecked(enabled);
}

void Form::on_pushButton_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->sendInjection(ui->comboBoxConnections->currentIndex(),ui->lineEditApplicationId->text(),ui->lineEditContextId->text(),ui->lineEditServiceId->text().toInt(),ui->lineEditData->text().toLatin1());
}

void Form::on_pushButtonJumpTo_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->jumpToMsg(ui->lineEditJumpTo->text().toUInt());
}

void Form::on_pushButtonClearFile_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->clearFile();
}

void Form::on_pushButtonNewFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("New DLT Log file"), tr(""), tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        return;
    }

    if(plugin->dltControl)
        plugin->dltControl->newFile(fileName);
}

void Form::on_pushButtonQuit_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->quitDltViewer();
}

void Form::on_pushButtonOpenFile_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Open one or more DLT Log files"), tr(""), tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileNames.isEmpty())
        return;

    if(plugin->dltControl)
        plugin->dltControl->openFile(fileNames);
}

void Form::on_pushButtonSaveAsFile_clicked()
{
    QFileDialog dialog(this);
    QStringList filters;
    filters << "DLT Files (*.dlt)" <<"All files (*.*)";
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("dlt");
    dialog.setNameFilters(filters);
    dialog.setWindowTitle("Save DLT Log file");
    dialog.exec();
    if(dialog.result() != QFileDialog::Accepted ||
        dialog.selectedFiles().count() < 1)
    {
        return;
    }

    QString fileName = dialog.selectedFiles()[0];

    if(fileName.isEmpty() || dialog.result() == QDialog::Rejected)
    {
        return;
    }

    if(plugin->dltControl)
        plugin->dltControl->saveAsFile(fileName);
}

void Form::on_pushButtonMarker_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->marker();
}

void Form::on_pushButtonConnect_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->connectEcu(ui->comboBoxConnections->currentIndex());
}

void Form::on_pushButtonDisconnect_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->disconnectEcu(ui->comboBoxConnections->currentIndex());
}


void Form::on_pushButtonReopenFile_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->reopenFile();
}
