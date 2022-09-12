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
 * \file testviewdialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "textviewdialog.h"
#include "ui_textviewdialog.h"
#include <QDebug>
TextviewDialog::TextviewDialog(QString file,  QByteArray *data,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextviewDialog)
{
    ui->setupUi(this);
    setWindowTitle("Preview of "+file);
    ui->textBrowser->setText(*data);
    connect(ui->printButton, SIGNAL(clicked()),this, SLOT(print()));
}

TextviewDialog::~TextviewDialog()
{
    delete ui;
}

void TextviewDialog::print(){
        QPrintDialog dialog(&printer, this);
        if (dialog.exec()) {
             ui->textBrowser->print(&printer);
        }
}
