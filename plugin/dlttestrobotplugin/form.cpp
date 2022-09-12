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
#include "dlttestrobotplugin.h"

#include <qfiledialog.h>

using namespace DltTestRobot;

Form::Form(DltTestRobotPlugin *_plugin,QWidget *parent) :
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

void Form::on_pushButtonStart_clicked()
{
    plugin->start();
}

void Form::on_pushButtonStop_clicked()
{
    plugin->stop();
}

void Form::start()
{
    plugin->setPort(ui->lineEditTCPPort->text().toUInt());

    ui->pushButtonStart->setDisabled(true);
    ui->pushButtonStop->setDisabled(false);
}

void Form::stop()
{
    ui->pushButtonStart->setDisabled(false);
    ui->pushButtonStop->setDisabled(true);
}

void Form::status(QString text)
{
    // status from DLT Mini Server
    ui->lineEditStatus->setText(text);

    // status of DLT communication changed
    if(text == "" || text == "stopped")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::white);
        ui->lineEditStatus->setPalette(palette);
    }
    else if(text == "listening")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::yellow);
        ui->lineEditStatus->setPalette(palette);
    }
    else if(text == "connected")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditStatus->setPalette(palette);
    }
    else if(text == "error")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditStatus->setPalette(palette);
    }
}
