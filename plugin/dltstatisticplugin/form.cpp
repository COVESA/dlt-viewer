/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file form.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "form.h"
#include "ui_form.h"
#include "dltstatisticplugin.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    timelineWidget = new QDltTimelineWidget(ui->tabTimeline);
    ui->tabTimeline->layout()->addWidget(timelineWidget);
    timelineWidget->show();
    ui->tabTimeline->setVisible(true);
    timelineWidget->setYUnitText("Msg");

    bandwidthWidget = new QDltTimelineWidget(ui->tabBandwidth);
    ui->tabBandwidth->layout()->addWidget(bandwidthWidget);
    bandwidthWidget->show();
    ui->tabBandwidth->setVisible(true);
    bandwidthWidget->setYUnitText("Bytes/s");
}

Form::~Form()
{
    delete ui;
}
void Form::update(time_t min,time_t max,QMap<time_t,int> &timeline,QMap<time_t,int> &bandwidth)
{
    timelineWidget->setBeginTime(min);
    timelineWidget->setEndTime(max);
    timelineWidget->setData(timeline);

    bandwidthWidget->setBeginTime(min);
    bandwidthWidget->setEndTime(max);
    bandwidthWidget->setData(bandwidth);
}

void Form::setTextBrowserOverview(QString text)
{
    if(text.isNull())
        ui->textBrowserOverview->setText("");
    else
        ui->textBrowserOverview->setText(text);

}
