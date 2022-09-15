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


//using namespace DltSpeedPlugin;

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    dataArray = new QwtPointArrayData(timeY,speedX);

    ui->qwtPlot->setTitle("MySpeedPlugin");

    ui->qwtPlot->setAxisScale(0,0,100);

    //ui->thermo->setMaximumHeight(100);
    //ui->thermo->setMinimumHeight(0);

    curve1 = new QwtPlotCurve("Curve 1");
    curve1->attach(ui->qwtPlot);

    QwtLinearColorMap * colorMap = new QwtLinearColorMap(Qt::green, Qt::red);
    colorMap->addColorStop(0, Qt::green);
    colorMap->addColorStop(80, Qt::red);
    ui->thermo->setColorMap(colorMap);
}

Form::~Form()
{
    delete ui;
}


void Form::setSpeedGraph(QDltArgument currentSpeed,unsigned int time)
{
    // Push data for speed and time to container
    speedX.push_back(currentSpeed.toString().toDouble());
    timeY.push_back((double)time);

    // Set currentSpeed to lcdNumber
    //ui->lcdNumber->display(currentSpeed.toString());

    // Copy the data into the curves
    dataArray = new QwtPointArrayData(timeY,speedX);
    curve1->setData(dataArray);

    // Change color if value is bigger than 80
    if(currentSpeed.toString().toDouble()<80)
    {
        curve1->setPen(QPen(Qt::green, 1));
    }
    else
    {
        curve1->setPen(QPen(Qt::red, 1));
    }


    //ui->thermo->setValue(currentSpeed.toString().toDouble());

    // Replot curve
    ui->qwtPlot->replot();

}

void Form::setSpeedLCD(QDltArgument currentSpeed,unsigned int time)
{
    // Set currentSpeed to lcdNumber
    ui->lcdNumber->display(currentSpeed.toString());

    ui->thermo->setValue(currentSpeed.toString().toDouble());
}

