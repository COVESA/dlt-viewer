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
 * \file pulsebutton.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "pulsebutton.h"
#include "QTimeLine"

PulseButton::PulseButton(QWidget *parent) :
    QPushButton(parent),
    animationTimeline(1000, parent)
{
    /* Set number of frames to 100.
     * Shape to curve that gives pleasant pulsing behaviour.
     * Loop forever */
    animationTimeline.setFrameRange(0, 100);
    
#ifdef QT5_QT6_COMPAT
    QEasingCurve easing(QEasingCurve::SineCurve);
    animationTimeline.setEasingCurve(easing);
#else
    animationTimeline.valueForTime(QEasingCurve::SineCurve);
#endif
    animationTimeline.setLoopCount(0);

    connect(&animationTimeline, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
    connect(&animationTimeline, SIGNAL(stateChanged(QTimeLine::State)), this, SLOT(animationStateChanged(QTimeLine::State)));

    /* Store the original background color */
    baseColor = this->palette().window().color();

    /* Let QT draw background whenever needed */
    setAutoFillBackground(true);
}

void PulseButton::startPulsing(QColor color)
{
    targetColor = color;
    animationTimeline.start();
}

void PulseButton::stopPulsing()
{
    animationTimeline.stop();
}

void PulseButton::frameChanged(int frame)
{
    /* Quantize integer to float 0..1 */
    float scale = (float)(frame)/100.0f;

    /* Get difference between the color components */
    int deltaR = targetColor.red() - baseColor.red();
    int deltaG = targetColor.green() - baseColor.green() ;
    int deltaB = targetColor.blue() - baseColor.blue();

    /* Scale the difference to 0..DeltaMax */
    deltaR *= scale;
    deltaG *= scale;
    deltaB *= scale;

    /* Sum the base color with the scaled difference */
    QColor newColor(
                baseColor.red() + deltaR,
                baseColor.green() + deltaG,
                baseColor.blue() + deltaB
                );

    /* Create new palette based on current, and replace the old one */
    QPalette newPalette(palette());
    newPalette.setColor(QPalette::Button, newColor);
    setPalette(newPalette);
}

void PulseButton::animationStateChanged(QTimeLine::State state)
{
    if(state == QTimeLine::NotRunning)
    {
        QPalette newPalette(palette());
        newPalette.setColor(QPalette::Button, baseColor);
        setPalette(newPalette);
    }
}
