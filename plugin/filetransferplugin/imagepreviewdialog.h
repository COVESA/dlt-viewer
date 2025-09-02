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
 * \file imagepreviewdialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef IMAGEPREVIEWDIALOG_H
#define IMAGEPREVIEWDIALOG_H

#include <QDialog>
#include <QScrollArea>
#include <QLabel>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QPrinter>
#include <QPrintDialog>

namespace Ui {
    class ImagePreviewDialog;
}

class ImagePreviewDialog : public QDialog
{
    Q_OBJECT

public:
    ImagePreviewDialog(QString file,  QByteArray *imageData, QWidget *parent = 0);
    ~ImagePreviewDialog();
    bool isFileSupported();

public slots:
    void print();
private:

     Ui::ImagePreviewDialog *ui;
     QGraphicsScene *scene;
     QPixmap pixmap;
     QGraphicsPixmapItem *pixmapItem;
     QPrinter printer;


    bool fileSupported;
};

#endif // IMAGEPREVIEWDIALOG_H
