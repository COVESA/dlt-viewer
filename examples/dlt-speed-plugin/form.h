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
#include "qwt_plot.h"
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_color_map.h>
#include <qwt_point_data.h>
#include "plugininterface.h"

//namespace DltSpeedPlugin {
    namespace Ui {
        class Form;
    }

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    void setSpeedLCD(QDltArgument currentSpeed,unsigned int time);
    void setSpeedGraph(QDltArgument currentSpeed,unsigned int time);

private:
    Ui::Form *ui;
    QwtPlotCurve *curve1;
    QwtPointArrayData *dataArray;

    QVector< double > speedX;
    QVector< double > timeY;
};

//} //namespace DummyViewer

#endif // FORM_H
