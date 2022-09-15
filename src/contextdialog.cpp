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
 * \file contextdialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "contextdialog.h"
#include "ui_contextdialog.h"

ContextDialog::ContextDialog(QString id,QString description,int loglevel, int tracestatus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContextDialog)
{
    ui->setupUi(this);

    ui->idLineEdit->setText(id);
    ui->descriptionLineEdit->setText(description);
    ui->loglevelComboBox->setCurrentIndex(loglevel+1);
    ui->tracestatusComboBox->setCurrentIndex(tracestatus+1);

}

ContextDialog::~ContextDialog()
{
    delete ui;
}

QString ContextDialog::id()
{
   return  ui->idLineEdit->text();
}

QString ContextDialog::description()
{
    return  ui->descriptionLineEdit->text();
}

int ContextDialog::loglevel()
{
    return  ui->loglevelComboBox->currentIndex()-1;
}

int ContextDialog::tracestatus()
{
    return  ui->tracestatusComboBox->currentIndex()-1;
}

int ContextDialog::update()
{
    return  ui->checkBoxUpdate->isChecked();
}
