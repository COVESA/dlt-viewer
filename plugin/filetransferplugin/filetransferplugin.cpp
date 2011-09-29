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
    form->makeConnections();

    fModel = new FileModel(form);
    return form;
}

bool FiletransferPlugin::initFile(QDltFile *file) {
    dltFile = file;
    updateFile();
    return true;
}

void FiletransferPlugin::updateFile() {
    QByteArray buffer;
    QDltMsg msg;
    QDltArgument protocolSFlag;
    QDltArgument protocolEFlag;

    if(!dltFile)
        return;

    for(;msgIndex<dltFile->size();msgIndex++)
    {
        buffer =  dltFile->getMsg(msgIndex);

        if(buffer.isEmpty())
            break;

        msg.setMsg(buffer);

        msg.getArgument(PROTOCOL_ALL_SFLAG,protocolSFlag);

        if(protocolSFlag.toString().compare("FLST") == 0 )
        {
            msg.getArgument(PROTOCOL_FLST_EFLAG,protocolEFlag);
            if(protocolEFlag.toString().compare("FLST") == 0)
            {
                fModel->doFLST(&msg);
            }
        } else if(protocolSFlag.toString().compare("FLDA") == 0 ) {
            msg.getArgument(PROTOCOL_FLDA_EFLAG,protocolEFlag);
            if(protocolEFlag.toString().compare("FLDA") == 0)
            {
                fModel->doFLDA(&msg);
            }
        } else if(protocolSFlag.toString().compare("FLFI") == 0 ) {
            msg.getArgument(PROTOCOL_FLFI_EFLAG,protocolEFlag);
            if(protocolEFlag.toString().compare("FLFI") == 0)
            {
                fModel->doFLFI(&msg);
            }
        } else if(protocolSFlag.toString().compare("FLIF") == 0 ) {
            msg.getArgument(PROTOCOL_FLIF_EFLAG,protocolEFlag);
            if(protocolEFlag.toString().compare("FLFI") == 0)
            {
                fModel->doFLIF(&msg);
            }
        } else if (protocolSFlag.toString().compare("FLER") == 0 ) {
            msg.getArgument(PROTOCOL_FLER_EFLAG,protocolEFlag);
            if(protocolEFlag.toString().compare("FLER") == 0)
            {
                fModel->doFLER(&msg);
            }
        }


    }

}

void FiletransferPlugin::selectedIdxMsg(int index) {
    if(!dltFile)
        return;
}

Q_EXPORT_PLUGIN2(filetransferplugin, FiletransferPlugin);
