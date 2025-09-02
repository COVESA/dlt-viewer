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
 * \file textviewdialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef TEXTVIEWDIALOG_H
#define TEXTVIEWDIALOG_H

#include <QDialog>
#include <QPrinter>
#include <QPrintDialog>
namespace Ui {
    class TextviewDialog;
}

class TextviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextviewDialog(QString file,  QByteArray *data, QWidget *parent = 0);
    ~TextviewDialog();
public slots:
    void print();
private:
    Ui::TextviewDialog *ui;
    QPrinter printer;
};

#endif // TEXTVIEWDIALOG_H
