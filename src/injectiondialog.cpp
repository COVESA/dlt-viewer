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
 * \file injectiondialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "injectiondialog.h"
#include "ui_injectiondialog.h"

#include "qdltsettingsmanager.h"

InjectionDialog::InjectionDialog(QString appid,QString conid,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InjectionDialog)
{
    ui->setupUi(this);
    ui->applicationidComboBox->setEditText(appid);
    ui->contextidComboBox->setEditText(conid);
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

void InjectionDialog::setApplicationId(QString text) { ui->applicationidComboBox->setEditText(text); }
void InjectionDialog::setContextId(QString text) { ui->contextidComboBox->setEditText(text); }
void InjectionDialog::setServiceId(QString text) { ui->serviceidComboBox->setEditText(text); }
void InjectionDialog::setData(QString text) { ui->dataComboBox->setEditText(text); }
void InjectionDialog::setDataBinary(bool mode) { ui->binaryRadioButton->setChecked(mode);ui->textRadioButton->setChecked(!mode); }

QString InjectionDialog::getApplicationId() { return ui->applicationidComboBox->currentText(); }
QString InjectionDialog::getContextId() { return ui->contextidComboBox->currentText(); }
QString InjectionDialog::getServiceId() { return ui->serviceidComboBox->currentText(); }
QString InjectionDialog::getData() { return ui->dataComboBox->currentText(); }
bool InjectionDialog::getDataBinary() { return ui->binaryRadioButton->isChecked(); }

void InjectionDialog::updateHistory()
{
    QStringList list;
    list = QDltSettingsManager::getInstance()->value("injection/applicationid").toStringList();
    ui->applicationidComboBox->clear();
    foreach(QString text,list)
       ui->applicationidComboBox->addItem(text);

    list = QDltSettingsManager::getInstance()->value("injection/contextid").toStringList();
    ui->contextidComboBox->clear();
    foreach(QString text,list)
       ui->contextidComboBox->addItem(text);

    list = QDltSettingsManager::getInstance()->value("injection/serviceid").toStringList();
    ui->serviceidComboBox->clear();
    foreach(QString text,list)
       ui->serviceidComboBox->addItem(text);

    list = QDltSettingsManager::getInstance()->value("injection/data").toStringList();
    ui->dataComboBox->clear();
    foreach(QString text,list)
       ui->dataComboBox->addItem(text);

}

void InjectionDialog::storeHistory()
{
    QStringList list;

    list = QDltSettingsManager::getInstance()->value("injection/applicationid").toStringList();
    list.removeAll(getApplicationId());
    list.prepend(getApplicationId());
    while (list.size() > INJECTION_MAX_HISTORY)
        list.removeLast();
    QDltSettingsManager::getInstance()->setValue("injection/applicationid",list);

    list = QDltSettingsManager::getInstance()->value("injection/contextid").toStringList();
    list.removeAll(getContextId());
    list.prepend(getContextId());
    while (list.size() > INJECTION_MAX_HISTORY)
        list.removeLast();
    QDltSettingsManager::getInstance()->setValue("injection/contextid",list);

    list = QDltSettingsManager::getInstance()->value("injection/serviceid").toStringList();
    list.removeAll(getServiceId());
    list.prepend(getServiceId());
    while (list.size() > INJECTION_MAX_HISTORY)
        list.removeLast();
    QDltSettingsManager::getInstance()->setValue("injection/serviceid",list);

    list = QDltSettingsManager::getInstance()->value("injection/data").toStringList();
    list.removeAll(getData());
    list.prepend(getData());
    while (list.size() > INJECTION_MAX_HISTORY)
        list.removeLast();
    QDltSettingsManager::getInstance()->setValue("injection/data",list);

}

