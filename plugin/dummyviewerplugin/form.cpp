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

void Form::setMessages(int message)
{
    ui->lineEditMessages->setText(QString("%1").arg(message));
}

void Form::setSelectedMessage(int message)
{
    ui->lineEditSelectedMessage->setText(QString("%1").arg(message));
}

void Form::setVerboseMessages(int message)
{
    ui->lineEditVerboseMessages->setText(QString("%1").arg(message));
}

void Form::setNonVerboseMessages(int message)
{
    ui->lineEditNonVerboseMessages->setText(QString("%1").arg(message));
}
