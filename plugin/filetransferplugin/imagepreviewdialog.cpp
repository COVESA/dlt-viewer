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
 * \file imagepreviewdialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "imagepreviewdialog.h"
#include "ui_imagepreviewdialog.h"

ImagePreviewDialog::ImagePreviewDialog(QString file, QByteArray *imageData, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ImagePreviewDialog)
{
    ui->setupUi(this);

    if(pixmap.loadFromData(*imageData))
    {
        scene = new QGraphicsScene(ui->graphicsView);
        scene->setBackgroundBrush(Qt::lightGray);

        setWindowTitle("Preview of "+file);
        pixmapItem = scene->addPixmap(pixmap);
        pixmapItem->setPixmap(pixmap);
        ui->graphicsView->setScene(scene);
        ui->graphicsView->centerOn(pixmapItem);
        fileSupported = true;

        connect(ui->printButton, SIGNAL(clicked()),this, SLOT(print()));

    }
    else
    {
          fileSupported = false;
    }
}

ImagePreviewDialog::~ImagePreviewDialog()
{
    delete ui;
}

bool ImagePreviewDialog::isFileSupported(){
    return fileSupported;
}

void ImagePreviewDialog::print(){
        QPrintDialog dialog(&printer, this);
        if (dialog.exec()) {
            QPainter painter(&printer);
            QRect rect = painter.viewport();
            QSize size = pixmap.size();
            size.scale(rect.size(), Qt::KeepAspectRatio);
            painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
            painter.setWindow(pixmap.rect());
            painter.drawPixmap(0, 0, pixmap);
        }

}
