#include "form.h"
#include "ui_form.h"
#include <QFileDialog>
#include <QMessageBox>
#include "imagepreviewdialog.h"
#include "textviewdialog.h"
#include <QtDebug>

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

void Form::makeConnections(){

    connect(ui->expandButton, SIGNAL(clicked()),this, SLOT(expandClicked()));
    connect(ui->collapseButton, SIGNAL(clicked()),this, SLOT(collapseClicked()));

    connect(ui->selectButton, SIGNAL(clicked()),this, SLOT(selectAllClicked()));
    connect(ui->deselectButton, SIGNAL(clicked()),this, SLOT(deselectAllClicked()));
    connect(ui->saveButton, SIGNAL(clicked()),this, SLOT(saveClicked()));

    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),this, SLOT(itemChanged(QTreeWidgetItem*,int)));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));

}

QTreeWidget* Form::getTreeWidget(){
    return ui->treeWidget;
}

void Form::expandClicked(){
    ui->treeWidget->expandAll();
}
void Form::collapseClicked(){
    ui->treeWidget->collapseAll();
}

void Form::selectAllClicked(){
    QTreeWidgetItemIterator it(ui->treeWidget,QTreeWidgetItemIterator::NotChecked|QTreeWidgetItemIterator::NoChildren);
    while (*it) {
        File *tmp = dynamic_cast<File*>(*it);

        if (tmp != NULL) {
            if(tmp->isComplete())
            {
                tmp->setCheckState(COLUMN_CHECK, Qt::Checked);
            }
        }
        ++it;
    }
}
void Form::deselectAllClicked(){
    QTreeWidgetItemIterator it(ui->treeWidget,QTreeWidgetItemIterator::NoChildren );//| QTreeWidgetItemIterator::Checked);

    while (*it) {
        File *tmp = dynamic_cast<File*>(*it);

        if (tmp != NULL) {

            if(tmp->isComplete())
            {
                tmp->setCheckState(COLUMN_CHECK, Qt::Unchecked);

            }
        }
        ++it;
    }
}
void Form::itemChanged(QTreeWidgetItem* item,int i){

    if(i == COLUMN_CHECK){
        File *tmp = dynamic_cast<File*>(item);

        if (tmp != NULL) {

            if(tmp->isComplete() && (tmp->checkState(COLUMN_CHECK) == Qt::Unchecked) )
            {
                tmp->setCheckState(COLUMN_CHECK, Qt::Checked);
            }
            else{
                tmp->setCheckState(COLUMN_CHECK, Qt::Unchecked);
            }
        }
    }

}

void Form::saveClicked(){
    QTreeWidgetItemIterator it(ui->treeWidget,QTreeWidgetItemIterator::NoChildren );//| QTreeWidgetItemIterator::Checked);

    while (*it) {
        File *tmp = dynamic_cast<File*>(*it);

        if (tmp != NULL) {

            if(tmp->isComplete() && (tmp->checkState(COLUMN_CHECK) == Qt::Checked) )
            {

                QString path = QFileDialog::getSaveFileName(this, tr("Save File"),QDir::homePath()+"//"+tmp->getFilename());

                QString text;
                QString infoText;
                QString detailedText;
                QMessageBox msgBox;

                if(path != NULL && tmp->saveFile(path)){
                    text += ("Save successful");
                    infoText += ("The file \n");
                    infoText += tmp->getFilenameOnTarget();
                    infoText += "\nwas saved to \n";
                    infoText += path;

                    msgBox.setIcon(QMessageBox::Information);

                } else {

                    text += ("Save incomplete");
                    infoText +=("The file \n");
                    infoText += tmp->getFilenameOnTarget();
                    infoText += "\nwas not saved.";

                    msgBox.setIcon(QMessageBox::Critical);
                }
                msgBox.setText(text);
                detailedText += ("Filename on target: ");
                detailedText += tmp->getFilenameOnTarget();
                detailedText += "\nFilename on host: ";
                detailedText += tmp->getFilename();
                detailedText += "\nPath on host: ";
                detailedText += path;
                detailedText += "\nFile serialnumber: ";
                detailedText += tmp->getFileSerialNumber();
                detailedText += "\nFilesize in Bytes: ";
                detailedText += QString::number(tmp->getSizeInBytes());
                detailedText += "\nPackages: ";
                detailedText += QString::number(tmp->getPackages());
                detailedText += "\nBuffersize in Bytes: ";
                detailedText += QString::number(tmp->getBufferSize());
                detailedText += "\nWritten Bytes: ";
                detailedText += QString::number(tmp->getWrittenBytes());

                msgBox.setInformativeText(infoText);
                msgBox.setDetailedText(detailedText);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.exec();

            }
        }

        ++it;
    }
}
void Form::itemDoubleClicked ( QTreeWidgetItem * item, int column ){

    File *tmp = dynamic_cast<File*>(item);
    if (tmp != NULL && tmp->isComplete())
    {
        ImagePreviewDialog img(tmp->getFilenameOnTarget(),tmp->getData(),this);
        if(img.isFileSupported())
        {
            if(img.exec()==1)
            {

            }
            else
            {

            }
        }
        else
        {
            TextviewDialog text(tmp->getFilenameOnTarget(),tmp->getData(),this);
            text.exec();

            //QMessageBox::information(this,"Image Preview","Could not open file for preview. File format not supported.");

        }
    }


}
