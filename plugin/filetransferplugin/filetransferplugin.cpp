#include <QtGui>
#include "filetransferplugin.h"

FiletransferPlugin::FiletransferPlugin() {
    dltFile = 0;
    msgIndex = 0;
}

FiletransferPlugin::~FiletransferPlugin() {

}

QString FiletransferPlugin::name() {
    return QString("Filetransfer Plugin");
}

QString FiletransferPlugin::pluginVersion()
{
    return FILETRANSFER_PLUGIN_VERSION;
}

QString FiletransferPlugin::pluginInterfaceVersion()
{
    return PLUGIN_INTERFACE_VERSION;
}

QString FiletransferPlugin::description() {
    QString description("This plugin enables the user to get a list of embedded files in a dlt log and save these files to disk. ");
    description += ("For more informations about this plugin please have a look on the dlt filetransfer documentation (generate it with doxygen).");
    return description;
}

QString FiletransferPlugin::error() {
    return QString();
}

bool FiletransferPlugin::loadConfig(QString /*filename*/) {

    return true;
}

bool FiletransferPlugin::saveConfig(QString /*filename*/) {

    return true;
}

QStringList FiletransferPlugin::infoConfig() {

    return QStringList();
}

QWidget* FiletransferPlugin::initViewer() {
    form = new Form();

    return form;
}

bool FiletransferPlugin::initFile(QDltFile *file) {
    dltFile = file;
    //msgIndex = 0;
    updateFile();
    return true;
}

void FiletransferPlugin::updateFile() {
    QByteArray buffer;
    QDltMsg msg;
    QDltArgument protocolStartFlag;
    QDltArgument protocolEndFlag;

    if(!dltFile)
        return;

    for(;msgIndex<dltFile->size();msgIndex++)
    {
        buffer =  dltFile->getMsg(msgIndex);

        if(buffer.isEmpty())
            break;

        msg.setMsg(buffer);

        msg.getArgument(PROTOCOL_ALL_STARTFLAG,protocolStartFlag);

        if(protocolStartFlag.toString().compare("FLST") == 0 )
        {
            msg.getArgument(PROTOCOL_FLST_ENDFLAG,protocolEndFlag);
            if(protocolEndFlag.toString().compare("FLST") == 0)
            {
                doFLST(&msg);
            }
        } else if(protocolStartFlag.toString().compare("FLDA") == 0 ) {
            msg.getArgument(PROTOCOL_FLDA_ENDFLAG,protocolEndFlag);
            if(protocolEndFlag.toString().compare("FLDA") == 0)
            {
                doFLDA(msgIndex,&msg);
            }
        } else if(protocolStartFlag.toString().compare("FLFI") == 0 ) {
            msg.getArgument(PROTOCOL_FLFI_ENDFLAG,protocolEndFlag);
            if(protocolEndFlag.toString().compare("FLFI") == 0)
            {
                doFLFI(&msg);
            }
        } else if(protocolStartFlag.toString().compare("FLIF") == 0 ) {
            msg.getArgument(PROTOCOL_FLIF_ENDFLAG,protocolEndFlag);
            if(protocolEndFlag.toString().compare("FLFI") == 0)
            {
                doFLIF(&msg);
            }
        } else if (protocolStartFlag.toString().compare("FLER") == 0 ) {
            msg.getArgument(PROTOCOL_FLER_ENDFLAG,protocolEndFlag);
            if(protocolEndFlag.toString().compare("FLER") == 0)
            {
                doFLER(&msg);
            }
        }


    }

}

void FiletransferPlugin::selectedIdxMsg(int index) {
    if(!dltFile)
        return;
}

void FiletransferPlugin::doFLST(QDltMsg *msg){

    QDltArgument argument;
    msg->getArgument(PROTOCOL_FLST_FILEID,argument);

    File *file = new File(dltFile,0);
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

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(file->getFileSerialNumber(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);

    if(result.isEmpty()){
        form->getTreeWidget()->addTopLevelItem(file);
    }
    else
    {
      int index = form->getTreeWidget()->indexOfTopLevelItem(result.at(0));
      form->getTreeWidget()->takeTopLevelItem(index);
      form->getTreeWidget()->addTopLevelItem(file);
    }

}

void FiletransferPlugin::doFLDA(int index,QDltMsg *msg){
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

            file->setQFileIndexForPackage(packageNumber.toString(),index);
        }
    }
}

void FiletransferPlugin::doFLFI(QDltMsg *msg){
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

void FiletransferPlugin::doFLIF(QDltMsg *msg){

}

void FiletransferPlugin::doFLER(QDltMsg *msg){
    QDltArgument filename;
    msg->getArgument(PROTOCOL_FLER_FILENAME,filename);
    QDltArgument errorCode1;
    msg->getArgument(PROTOCOL_FLER_ERRCODE1,errorCode1);
    QDltArgument errorCode2;
    msg->getArgument(PROTOCOL_FLER_ERRCODE2,errorCode2);

    File *file= new File(0);

    QList<QTreeWidgetItem *> result = form->getTreeWidget()->findItems(filename.toString(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILENAME);

    if(result.isEmpty()){
       form->getTreeWidget()->addTopLevelItem(file);
    }
    else
    {
       file = (File*)result.at(0);
       int index = form->getTreeWidget()->indexOfTopLevelItem(result.at(0));
       form->getTreeWidget()->takeTopLevelItem(index);
       form->getTreeWidget()->addTopLevelItem(file);
    }

    file->errorHappens(filename.toString(),errorCode1.toString(),errorCode2.toString(),msg->getTimeString());
    file->setFlags(Qt::NoItemFlags );
}

Q_EXPORT_PLUGIN2(filetransferplugin, FiletransferPlugin);
