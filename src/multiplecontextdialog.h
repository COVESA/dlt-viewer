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
 * \file multiplecontextdialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef MULTIPLECONTEXTDIALOG_H
#define MULTIPLECONTEXTDIALOG_H

#include <QDialog>

namespace Ui {
    class MultipleContextDialog;
}

class MultipleContextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultipleContextDialog(int loglevel, int tracestatus, QWidget *parent = 0);
    ~MultipleContextDialog();

    int loglevel();
    int tracestatus();
    int update();
private:
    Ui::MultipleContextDialog *ui;
};

#endif // MULTIPLECONTEXTDIALOG_H
