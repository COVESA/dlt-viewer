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
 * \file form.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef FORM_H
#define FORM_H

#include <QWidget>


namespace DltViewer {
    namespace Ui {
        class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    void setTextBrowserDetails(QString text);
    void setTextBrowserMessage(QString text);
    void setTextBrowserAscii(QString text);
    void setTextBrowserBinary(QString text);
    void setTextBrowserMixed(QString text);
    void setTextBrowserUncoded(QString text);
    void setTextBrowserDltV2(QString text);

private:
    Ui::Form *ui;
};

} //namespace DltViewer

#endif // FORM_H
