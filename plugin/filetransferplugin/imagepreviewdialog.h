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
