/**
 * @licence app begin@
 * Copyright (C) 2013  BMW AG
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
 * \file pulsebutton.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef PULSEBUTTON_H
#define PULSEBUTTON_H

#include <QPushButton>
#include <QTimeLine>

class PulseButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PulseButton(QWidget *parent = 0);
    
signals:
    
public slots:
    void startPulsing(QColor color);
    void stopPulsing();
    void frameChanged(int frame);
    void animationStateChanged(QTimeLine::State state);
private:
    QTimeLine animationTimeline;
    QColor baseColor;
    QColor targetColor;
};

#endif // PULSEBUTTON_H
