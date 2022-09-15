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
 * \file multiplecontextdialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "multiplecontextdialog.h"
#include "ui_multiplecontextdialog.h"

MultipleContextDialog::MultipleContextDialog(int loglevel, int tracestatus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultipleContextDialog)
{
    ui->setupUi(this);
    ui->loglevelComboBox->setCurrentIndex(loglevel);
    ui->tracestatusComboBox->setCurrentIndex(tracestatus);
}

MultipleContextDialog::~MultipleContextDialog()
{
    delete ui;
}

int MultipleContextDialog::loglevel()
{
    return  ui->loglevelComboBox->currentIndex()-1;
}

int MultipleContextDialog::tracestatus()
{
    return  ui->tracestatusComboBox->currentIndex()-1;
}

int MultipleContextDialog::update()
{
    return  ui->updateCheckBox->isChecked();
}
