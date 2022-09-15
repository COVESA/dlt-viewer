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
 * \file applicationdialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "applicationdialog.h"
#include "ui_applicationdialog.h"

ApplicationDialog::ApplicationDialog(QString id,QString description,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ApplicationDialog)
{
    ui->setupUi(this);

    ui->idLineEdit->setText(id);
    ui->descriptionLineEdit->setText(description);
}

ApplicationDialog::~ApplicationDialog()
{
    delete ui;
}

QString ApplicationDialog::id()
{
   return  ui->idLineEdit->text();
}

QString ApplicationDialog::description()
{
    return  ui->descriptionLineEdit->text();
}
