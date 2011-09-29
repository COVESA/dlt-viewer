#include "filemodel.h"
#include <iostream>
//filemodel::filemodel(QObject *parent, Form *f):
//    QObject(parent)
//{
//    form = f;
//}

FileModel::FileModel(){}
FileModel::FileModel(Form *f){
    form = f;
}
FileModel::~FileModel() {

}
void FileModel::doFLST(QDltMsg *msg){

    QDltArgument argument;
    msg->getArgument(PROTOCOL_FLST_FILEID,argument);

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(argument.toString(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);

    if(result.isEmpty()){
        QTreeWidgetItem *apId = 0;
        QTreeWidgetItem *ctId = 0;

        QTreeWidgetItem *tmpWidgetItem = 0;
        QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(msg->getApid(),Qt::MatchExactly,COLUMN_APID);
        if(result.isEmpty())
        {
            apId = new QTreeWidgetItem(form->getTreeWidget());
            apId->setText(COLUMN_APID, msg->getApid());
            ctId = new QTreeWidgetItem(apId);
            ctId->setText(COLUMN_CTID, msg->getCtid());
        }else{
            apId = result.at(0);
            int childCount = apId->childCount();
            for(int i=0; i<childCount;i++){
                tmpWidgetItem = apId->child(i);
                if(tmpWidgetItem->text(COLUMN_CTID).compare(msg->getCtid()) == 0){
                    ctId = apId->child(i);
                }else{
                    ctId = new QTreeWidgetItem(apId);
                    ctId->setText(COLUMN_CTID, msg->getCtid());
                }
            }
        }

        //Item can not be selected
        apId->setFlags(Qt::ItemIsEnabled);
        ctId->setFlags( Qt::ItemIsEnabled);

        File *file = new File(ctId);
        file->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        file->setCheckState(COLUMN_CHECK, Qt::Unchecked);

        msg->getArgument(PROTOCOL_FLST_FILEID,argument);
        file->setFileSerialNumber(argument.toString());

        msg->getArgument(PROTOCOL_FLST_FILENAME,argument);
        file->setFilename(argument.toString());

        msg->getArgument(PROTOCOL_FLST_FILEDATE,argument);
        file->setFileCreationDate(argument.toString());

        msg->getArgument(PROTOCOL_FLST_SIZE,argument);
        file->setSizeInBytes(argument.toString());

        msg->getArgument(PROTOCOL_FLST_PACKAGES,argument);
        file->setPackages(argument.toString());

        msg->getArgument(PROTOCOL_FLST_BUFFERSIZE,argument);
        file->setBuffersize(argument.toString());


        //items.append(apId);
        //form->setQTreeWidgetItems(items);

    }
    else
    {
        //Transfer for this file already started.
    }
}


void FileModel::doFLDA(QDltMsg *msg){
    QDltArgument argument;
    msg->getArgument(PROTOCOL_FLDA_FILEID,argument);

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(argument.toString(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);

    if(result.isEmpty())
    {
        //Transfer for this file started before sending FLST
    }
    else
    {
        File *file = (File*)result.at(0);
        if(!file->isComplete())
        {
            QDltArgument packageNumber;
            msg->getArgument(PROTOCOL_FLDA_PACKAGENR,packageNumber);

            QDltArgument data;
            msg->getArgument(PROTOCOL_FLDA_DATA,data);

            file->appendData(packageNumber.toString(),data.getDataSize(),data.getData());
        }
    }

}
void FileModel::doFLFI(QDltMsg *msg){
    QDltArgument argument;
    msg->getArgument(PROTOCOL_FLFI_FILEID,argument);

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(argument.toString(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);

    if(result.isEmpty())
    {
        //Transfer for this file started before sending FLST
    }
    else
    {
        File *file = (File*)result.at(0);
        if(file->isComplete())
        {
            file->setComplete();
        }
    }
}
void FileModel::doFLIF(QDltMsg *msg){

}

void FileModel::doFLER(QDltMsg *msg){
     QDltArgument filename;
    msg->getArgument(PROTOCOL_FLER_FILENAME,filename);
    QDltArgument errorCode1;
    msg->getArgument(PROTOCOL_FLER_ERRCODE1,errorCode1);
    QDltArgument errorCode2;
    msg->getArgument(PROTOCOL_FLER_ERRCODE2,errorCode2);

    File *file;


    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(filename.toString(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_APID);

    if(result.isEmpty()){
        QTreeWidgetItem *apId;
        QTreeWidgetItem *ctId;

        QTreeWidgetItem *tmpWidgetItem = 0;
        QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(msg->getApid(),Qt::MatchExactly,COLUMN_APID);
        if(result.isEmpty())
        {
            apId = new QTreeWidgetItem(form->getTreeWidget());
            apId->setText(COLUMN_APID, msg->getApid());
            ctId = new QTreeWidgetItem(apId);
            ctId->setText(COLUMN_CTID, msg->getCtid());
        }else{
            apId = result.at(0);
            int childCount = apId->childCount();
            for(int i=0; i<childCount;i++){
                tmpWidgetItem = apId->child(i);
                if(tmpWidgetItem->text(COLUMN_CTID).compare(msg->getCtid()) == 0){
                    ctId = apId->child(i);
                }else{
                    ctId = new QTreeWidgetItem(apId);
                    ctId->setText(COLUMN_CTID, msg->getCtid());
                }
            }
        }

        //Item can not be selected
        apId->setFlags(Qt::ItemIsEnabled);
        ctId->setFlags( Qt::ItemIsEnabled);

       file= new File(ctId);
    }
    else
    {
       file = (File*)result.at(0);
    }
    file->errorHappens(filename.toString(),errorCode1.toString(),errorCode2.toString(),msg->getTime().toString());
    file->setFlags(Qt::NoItemFlags );
    //file->setCheckState(COLUMN_CHECK, Qt::Unchecked);
}


