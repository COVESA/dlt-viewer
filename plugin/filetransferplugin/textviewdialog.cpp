#include "textviewdialog.h"
#include "ui_textviewdialog.h"

TextviewDialog::TextviewDialog(QString file,  QByteArray data,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextviewDialog)
{
    ui->setupUi(this);


    setWindowTitle("Preview of "+file);

    ui->textBrowser->setText(data);

    connect(ui->printButton, SIGNAL(clicked()),this, SLOT(print()));
}

TextviewDialog::~TextviewDialog()
{
    delete ui;
}

void TextviewDialog::print(){
        QPrintDialog dialog(&printer, this);
        if (dialog.exec()) {
             ui->textBrowser->print(&printer);
        }
}
