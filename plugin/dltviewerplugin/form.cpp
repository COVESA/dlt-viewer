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

using namespace DltViewer;

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

void Form::setTextBrowserDetails(QString text)
{
    ui->textBrowserDetails->setText(text);
}

void Form::setTextBrowserMessage(QString text)
{
    text.replace(QString("\n"), QString("<br>"));
    ui->textBrowserMessage->setText(text);
}

void Form::setTextBrowserAscii(QString text)
{
    ui->textBrowserAscii->setText(text);
}

void Form::setTextBrowserBinary(QString text)
{
    ui->textBrowserBinary->setText(text);
}

void Form::setTextBrowserMixed(QString text)
{
    ui->textBrowserMixed->setText(text);
}

void Form::setTextBrowserUncoded(QString text)
{
    ui->textBrowserUncoded->setText(text);
}

void Form::setTextBrowserDltV2(QString text)
{
    ui->textBrowserDltV2->setText(text);
}
