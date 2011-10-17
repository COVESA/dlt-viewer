#ifndef TEXTVIEWDIALOG_H
#define TEXTVIEWDIALOG_H

#include <QDialog>
#include <QPrinter>
#include <QPrintDialog>
namespace Ui {
    class TextviewDialog;
}

class TextviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextviewDialog(QString file,  QByteArray *data, QWidget *parent = 0);
    ~TextviewDialog();
public slots:
    void print();
private:
    Ui::TextviewDialog *ui;
    QPrinter printer;
};

#endif // TEXTVIEWDIALOG_H
