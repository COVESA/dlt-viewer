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
 * \file form.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef FORM_H
#define FORM_H

#include <QWidget>

namespace DltDbus {
namespace Ui {
    class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    void setTextBrowserDBus(QString text);
    void setTextBrowserHeader(QString text);
    void setTextBrowserPayload(QString text);
    void setTextBrowserPayloadHex(QString text);

private:
    Ui::Form *ui;
};

} //namespace DltDbus

#endif // FORM_H
