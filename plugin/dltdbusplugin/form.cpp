/**
 * @licence app begin@
 * Copyright (C) 2011-2014  BMW AG
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

using namespace DltDbus;

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
}

Form::~Form()
{
    delete ui;
}

void Form::setTextBrowserDBus(QString text)
{
    ui->textBrowserDBus->setText(text);
}

void Form::setTextBrowserHeader(QString text)
{
    ui->textBrowserHeader->setText(text);
}

void Form::setTextBrowserPayload(QString text)
{
    ui->textBrowserPayload->setText(text);
}

void Form::setTextBrowserPayloadHex(QString text)
{
    ui->textBrowserPayloadHex->setText(text);
}
