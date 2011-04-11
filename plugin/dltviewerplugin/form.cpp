#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
}

Form::~Form()
{
    delete ui;
}

void Form::setTextBrowserHeader(QString text)
{
    ui->textBrowserHeader->setText(text);
}

void Form::setTextBrowserPayload(QString text)
{
    ui->textBrowserPayload->setText(text);
}

void Form::setTextBrowserAscii(QString text)
{
    ui->textBrowserAscii->setText(text);
}

void Form::setTextBrowserBinary(QString text)
{
    ui->textBrowserBinary->setText(text);
}

void Form::setTextBrowserMixed(QString text)
{
    ui->textBrowserMixed->setText(text);
}

