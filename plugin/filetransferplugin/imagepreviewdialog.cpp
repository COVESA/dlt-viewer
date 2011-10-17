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
