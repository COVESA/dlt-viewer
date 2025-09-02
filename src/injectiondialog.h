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
 * \file injectiondialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef INJECTIONDIALOG_H
#define INJECTIONDIALOG_H

#include <QDialog>

namespace Ui {
    class InjectionDialog;
}

#define INJECTION_MAX_HISTORY 10

class InjectionDialog : public QDialog {
    Q_OBJECT
public:
    InjectionDialog(QString appid,QString conid,QWidget *parent = 0);
    ~InjectionDialog();

    void setApplicationId(QString text);
    void setContextId(QString text);
    void setServiceId(QString text);
    void setData(QString text);
    void setDataBinary(bool mode);

    QString getApplicationId();
    QString getContextId();
    QString getServiceId();
    QString getData();
    bool getDataBinary();

    void updateHistory();
    void storeHistory();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::InjectionDialog *ui;
};

#endif // INJECTIONDIALOG_H
