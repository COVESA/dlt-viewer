#include <QtGui>

#include "dltviewerplugin.h"

DltViewerPlugin::DltViewerPlugin() {
    dltFile = 0;
}

DltViewerPlugin::~DltViewerPlugin() {

}

QString DltViewerPlugin::name() {
    return QString("DLT Viewer Plugin");
}

QString DltViewerPlugin::pluginVersion(){
    return DLT_VIEWER_PLUGIN_VERSION;
}

QString DltViewerPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DltViewerPlugin::description() {
    return QString();
}

QString DltViewerPlugin::error() {
    return QString();
}

bool DltViewerPlugin::loadConfig(QString /*filename*/) {
    return true;
}

bool DltViewerPlugin::saveConfig(QString /*filename*/) {
    return true;
}

QStringList DltViewerPlugin::infoConfig() {
    return QStringList();
}

QWidget* DltViewerPlugin::initViewer() {
    form = new Form();
    return form;
}

bool DltViewerPlugin::initFile(QDltFile *file) {
    dltFile = file;

    return true;
}

void DltViewerPlugin::updateFile() {
    if(!dltFile)
        return;

}

void DltViewerPlugin::selectedIdxMsg(int index) {
    QString text;
    QDltMsg msg;
    QDltArgument argument;
    QByteArray buf;

    if(!dltFile)
        return;

    /* get message */
    buf = dltFile->getMsgFilter(index);
    if(buf.isEmpty())
        return;
    msg.setMsg(buf);

    //if(dltFile->getMsg(index,msg)==false)
    //    return;

    QByteArray bytes = msg.getPayload();

    /* Show Header */
    text = QString("<html><body>");
    text += QString("<table border=\"1\">");
    text += QString("<tr><th>Index</th><th>Time</th><th>Timestamp</th><th>Count</th><th>Ecuid</th><th>Apid</th><th>Ctid</th>");
    text += QString("<th>Type</th><th>Subtype</th><th>Mode</th><th>Endianness</th><th>#Args</th></tr>");
    text += QString("<tr><td>%1</td>").arg(dltFile->getMsgFilterPos(index));
    text += QString("<td>%1.%2</td>").arg(msg.getTime().toString("yyyy/MM/dd hh:mm:ss")).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
    text += QString("<td>%1.%2</td>").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
    text += QString("<td>%1</td>").arg(msg.getMessageCounter());
    text += QString("<td>%1</td>").arg(msg.getEcuid());
    text += QString("<td>%1</td>").arg(msg.getApid());
    text += QString("<td>%1</td>").arg(msg.getCtid());
    text += QString("<td>%1</td>").arg(msg.getTypeString());
    text += QString("<td>%1</td>").arg(msg.getSubtypeString());
    text += QString("<td>%1</td>").arg(msg.getModeString());
    text += QString("<td>%1</td>").arg(msg.getEndiannessString());
    text += QString("<td>%1</td>").arg(msg.getNumberOfArguments());
    text += QString("</tr></table>");
    text += QString("</body></html>");
    form->setTextBrowserHeader(text);

    /* Show payload */
    text = QString("<html><body>");

    text += QString("<table border=\"1\">");
    text += QString("<tr><th>MessageId</th><th>CtrlServiceId</th><th>CtrlReturnType</th></tr>");
    text += QString("<tr>");
    text += QString("<td>%1</td>").arg(msg.getMessageId());
    text += QString("<td>%1</td>").arg(msg.getCtrlServiceIdString());
    text += QString("<td>%1</td>").arg(msg.getCtrlReturnTypeString());
    text += QString("</tr></table>");

    text += QString("<table border=\"1\">");
    text += QString("<tr><th>Index</th><th>Offest</th><th>Size</th><th>TypeInfo</th><th>Name</th><th>Unit</th><th>Text</th><th>Binary</th></tr>");
    for(int num=0;num<msg.getNumberOfArguments();num++) {
        text += QString("<tr>");
        if(msg.getArgument(num,argument)) {
            text += QString("<td>%1</td>").arg(num+1);
            text += QString("<td>%1</td>").arg(argument.getOffsetPayload());
            text += QString("<td>%1</td>").arg(argument.getDataSize());
            text += QString("<td>%1</td>").arg(argument.getTypeInfoString());
            text += QString("<td>%1</td>").arg(argument.getName());
            text += QString("<td>%1</td>").arg(argument.getUnit());
            text += QString("<td>");

            // Necessary to display < and > as characters in a HTML context.
            // Otherwise < and > would be handled as HTML tags and not the complete payload would be displayed.
            QString payloadText = argument.toString(); /* text output */
            payloadText = payloadText.replace("<","&#60;");
            payloadText = payloadText.replace(">","&#62;");
            text += payloadText;

            text += QString("</td>");
            text += QString("<td>");
            text += argument.toString(true); /* Binary output */
            text += QString("</td>");
        }
        text += QString("</tr>");
    }
    text += QString("</table>");
    text += QString("</body></html>");
    form->setTextBrowserPayload(text);

    /* Show Ascii output */
    form->setTextBrowserAscii(msg.toAsciiTable(bytes,false,false,true,8,64));

    /* Show Binary output */
    form->setTextBrowserBinary(msg.toAsciiTable(bytes,true,true,false));

    /* Show Mixed output */
    form->setTextBrowserMixed(msg.toAsciiTable(bytes,true,true,true));
}

Q_EXPORT_PLUGIN2(dltviewerplugin, DltViewerPlugin);
