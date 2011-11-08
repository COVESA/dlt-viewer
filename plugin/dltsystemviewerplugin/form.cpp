#include "form.h"
#include "ui_form.h"
#include "dltsystemviewerplugin.h"

ProcessItem::ProcessItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent)
{
    lastTimestamp = 0;
}

ProcessItem::~ProcessItem()
{

}

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

void Form::addProcesses(int pid, QString data,QDltMsg &msg)
{
    QStringList datalist;
    ProcessItem *widget;
    int found = 0;
    int procent = 0;

    datalist = data.split(" ");

    for(int num=0;num<ui->treeWidget->topLevelItemCount();num++) {
        widget = (ProcessItem*) ui->treeWidget->topLevelItem(num);
        if(widget->text(0).toInt()==pid) {
            found = 1;
            //procent = datalist.at(13).toInt()-widget->text(2).toInt()+datalist.at(14).toInt()-widget->text(3).toInt();
            procent = ((datalist.at(13).toInt())-widget->text(2).toInt()+datalist.at(14).toInt()-widget->text(3).toInt())*10000/(msg.getTimestamp()-widget->lastTimestamp);///4;
            widget->setText(2,datalist.at(13));
            widget->setText(3,datalist.at(14));
            widget->setText(4,QString("%1").arg(procent));
            widget->lastTimestamp = msg.getTimestamp();
            break;
        }
    }

    if(!found) {
        widget = new ProcessItem();
        widget->setText(0,QString("%1").arg(pid));
        widget->setText(1,datalist.at(1));
        widget->setText(2,datalist.at(13));
        widget->setText(3,datalist.at(14));
        widget->setText(4,QString("%1").arg(procent));
        ui->treeWidget->insertTopLevelItem(0, widget);
    }
}

void Form::deleteAllProccesses()
{
    ui->treeWidget->clear();
}


void Form::on_pushButtonClear_clicked()
{
    ui->treeWidget->clear();
}

void Form::setUser(QString text)
{
    ui->lineEditUser->setText(text);
}

void Form::setNice(QString text)
{
    ui->lineEditNice->setText(text);
}

void Form::setSystem(QString text)
{
    ui->lineEditSystem->setText(text);
}
