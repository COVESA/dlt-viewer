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
 * \file <FILE>
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "injectiondialog.h"
#include "ui_injectiondialog.h"

InjectionDialog::InjectionDialog(QString appid,QString conid,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InjectionDialog)
{
    ui->setupUi(this);
    ui->applicationidLineEdit->setText(appid);
    ui->contextidLineEdit->setText(conid);
}

InjectionDialog::~InjectionDialog()
{
    delete ui;
}

void InjectionDialog::changeEvent(QEvent *e)
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

void InjectionDialog::setApplicationId(QString text) { ui->applicationidLineEdit->setText(text); }
void InjectionDialog::setContextId(QString text) { ui->contextidLineEdit->setText(text); }
void InjectionDialog::setServiceId(QString text) { ui->serviceidLineEdit->setText(text); }
void InjectionDialog::setData(QString text) { ui->dataLineEdit->setText(text); }

QString InjectionDialog::getApplicationId() { return ui->applicationidLineEdit->text(); }
QString InjectionDialog::getContextId() { return ui->contextidLineEdit->text(); }
QString InjectionDialog::getServiceId() { return ui->serviceidLineEdit->text(); }
QString InjectionDialog::getData() { return ui->dataLineEdit->text(); }
