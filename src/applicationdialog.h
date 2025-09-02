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
 * \file applicationdialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef APPLICATIONDIALOG_H
#define APPLICATIONDIALOG_H

#include <QDialog>

namespace Ui {
    class ApplicationDialog;
}

class ApplicationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApplicationDialog(QString id,QString description,QWidget *parent = 0);
    ~ApplicationDialog();

    QString id();
    QString description();

private:
    Ui::ApplicationDialog *ui;
};

#endif // APPLICATIONDIALOG_H
