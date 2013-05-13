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
 * \file form.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "form.h"
#include "ui_form.h"
#include "dummycontrolplugin.h"

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

void Form::on_pushButton_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->sendInjection(ui->comboBoxConnections->currentIndex(),ui->lineEditApplicationId->text(),ui->lineEditContextId->text(),ui->lineEditServiceId->text().toInt(),ui->lineEditData->text().toLatin1());
}
