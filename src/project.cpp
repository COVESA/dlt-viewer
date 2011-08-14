#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMessageBox>

#include "project.h"

const char *loginfo[] = {"default","off","fatal","error","warn","info","debug","verbose","","","","","","","","",""};
const char *traceinfo[] = {"default","off","on"};

EcuItem::EcuItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent,ecu_type)
{
    /* initialise receive buffer and message*/
    totalBytesRcvd = 0;

    tryToConnect = 0;
    connected = 0;

    interfacetype = 0; /* default TCP */
    loglevel = DLT_LOG_INFO;
    tracestatus = DLT_TRACE_STATUS_OFF;
    verbosemode = 1;
    status = EcuItem::unknown;

    this->sendSerialHeaderTcp = false;

    baudrate = 19; /* default 115200 */
    sendSerialHeaderSerial = true;

    serialport = 0;

    tcpport = DLT_DAEMON_TCP_PORT;

    /* Limit size of socket receiption buffer to limit application buffer size */
    /* qt sets buffer normally to unlimited */
    //socket.setReadBufferSize(64000);

    timingPackets = false;

    sendGetLogInfo = false;

    updateDataIfOnline = false;


}

EcuItem::~EcuItem()
{

}

void EcuItem::update()
{
    if(tryToConnect & connected)
    {
        setData(0,Qt::DisplayRole,id + " online");
        setBackground(0,QBrush(QColor(Qt::green)));
    }
    else if(tryToConnect & !connected)
    {
        if(connectError.isEmpty())
        {
            setData(0,Qt::DisplayRole,id + " connect");
            setBackground(0,QBrush(QColor(Qt::yellow)));
        }
        else
        {
            setData(0,Qt::DisplayRole,id + " connect ["+connectError+"]");
            setBackground(0,QBrush(QColor(Qt::red)));
        }

    }
    else
    {
        setData(0,Qt::DisplayRole,id + " offline");
        setBackground(0,QBrush(QColor(Qt::white)));
    }

    if(interfacetype == 0)
        setData(1,Qt::DisplayRole,QString("%1 [%2:%3]").arg(description).arg(hostname).arg(tcpport));
    else
        setData(1,Qt::DisplayRole,QString("%1 [%2]").arg(description).arg(port));

    setData(2,Qt::DisplayRole,QString("Default: %1").arg(loginfo[loglevel+1]));
    setData(3,Qt::DisplayRole,QString("Default: %1").arg(traceinfo[tracestatus+1]));

    if(status == EcuItem::invalid)
    {
        setBackground(2,QBrush(QColor(Qt::red)));
        setBackground(3,QBrush(QColor(Qt::red)));
    }
    else if(status == EcuItem::unknown)
    {
        setBackground(2,QBrush(QColor(Qt::yellow)));
        setBackground(3,QBrush(QColor(Qt::yellow)));
    }
    else if(status == EcuItem::valid)
    {
        setBackground(2,QBrush(QColor(Qt::green)));
        setBackground(3,QBrush(QColor(Qt::green)));
    }
}

void EcuItem::InvalidAll()
{
    status = EcuItem::unknown;
    update();

    for(int numapp = 0; numapp < childCount(); numapp++)
    {
        ApplicationItem * appitem = (ApplicationItem *) child(numapp);
        for(int numcontext = 0; numcontext < appitem->childCount(); numcontext++)
        {
            ContextItem * conitem = (ContextItem *) appitem->child(numcontext);

            conitem->status = ContextItem::unknown;
            conitem->update();

        }
    }

}

ApplicationItem::ApplicationItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent,application_type)
{

}

ApplicationItem::~ApplicationItem()
{

}

void ApplicationItem::update()
{
    setData(0,0,id);
    setData(1,0,description);
}

ContextItem::ContextItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent,context_type)
{
    loglevel = DLT_LOG_DEFAULT;
    tracestatus = DLT_TRACE_STATUS_DEFAULT;

    status = ContextItem::unknown;
}

ContextItem::~ContextItem()
{

}

void ContextItem::update()
{
    setData(0,0,id);
    setData(1,0,description);
    setData(2,0,QString("%1").arg(loginfo[loglevel+1]));
    setData(3,0,QString("%1").arg(traceinfo[tracestatus+1]));

    if(status == ContextItem::invalid)
    {
        setBackground(2,QBrush(QColor(Qt::red)));
        setBackground(3,QBrush(QColor(Qt::red)));
    }
    else if(status == ContextItem::unknown)
    {
        setBackground(2,QBrush(QColor(Qt::yellow)));
        setBackground(3,QBrush(QColor(Qt::yellow)));
    }
    else if(status == ContextItem::valid)
    {
        setBackground(2,QBrush(QColor(Qt::green)));
        setBackground(3,QBrush(QColor(Qt::green)));
    }
}

FilterItem::FilterItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent,filter_type)
{
    type = FilterItem::positive;

    name = "New Filter";

    enableEcuId = false;
    enableApplicationId = false;
    enableContextId = false;
    enableHeaderText = false;
    enablePayloadText = false;
    enableLogLevelMax = false;
    enableLogLevelMin = false;
    enableCtrlMsgs = false;

    filterColour = QColor();

    logLevelMax = 6;
    logLevelMin = 0;

}

FilterItem::~FilterItem()
{

}

void FilterItem:: operator = (FilterItem &item)
{
    type = item.type;

    name = item.name;
    ecuId = item.ecuId;
    applicationId = item.applicationId;
    contextId = item.contextId;
    headerText = item.headerText;
    payloadText = item.payloadText;

    enableEcuId = item.enableEcuId;
    enableApplicationId = item.enableApplicationId;
    enableContextId = item.enableContextId;
    enableHeaderText = item.enableHeaderText;
    enablePayloadText = item.enablePayloadText;
    enableLogLevelMax = item.enableLogLevelMax;
    enableLogLevelMin = item.enableLogLevelMin;
    enableCtrlMsgs = item.enableCtrlMsgs;

    filterColour = item.filterColour;

    logLevelMax = item.logLevelMax;
    logLevelMin = item.logLevelMin;

}


void FilterItem::update()
{
    QString text;

    switch(type)
    {
    case FilterItem::positive:
        text += QString("POSITIVE ");
        break;
    case FilterItem::negative:
        text += QString("NEGATIVE ");
        break;
    case FilterItem::marker:
        text += QString("MARKER ");
        break;
    }

    if(enableEcuId ) {
        text += QString("%1 ").arg(ecuId);
    }
    if(enableApplicationId ) {
        text += QString("%1 ").arg(applicationId);
    }
    if(enableContextId ) {
        text += QString("%1 ").arg(contextId);
    }
    if(enableHeaderText ) {
        text += QString("%1 ").arg(headerText);
    }
    if(enablePayloadText ) {
        text += QString("%1 ").arg(payloadText);
    }
    if(enableCtrlMsgs ) {
        text += QString("CtrlMsgs ").arg(payloadText);
    }
    if(enableLogLevelMax ) {
        switch(logLevelMax)
        {
        case 0:
            text += "off";
            break;
        case 1:
            text += "fatal";
            break;
        case 2:
            text += "error";
            break;
        case 3:
            text += "warn";
            break;
        case 4:
            text += "info";
            break;
        case 5:
            text += "debug";
            break;
        case 6:
            text += "verbose";
            break;
        default:
            break;
        }
        text += " ";
    }
    if(enableLogLevelMin ) {
        switch(logLevelMin)
        {
        case 0:
            text += "off";
            break;
        case 1:
            text += "fatal";
            break;
        case 2:
            text += "error";
            break;
        case 3:
            text += "warn";
            break;
        case 4:
            text += "info";
            break;
        case 5:
            text += "debug";
            break;
        case 6:
            text += "verbose";
            break;
        default:
            break;
        }
        text += " ";
    }
    if(type == FilterItem::marker)
    {
        text += filterColour.name();
    }

    if(text.isEmpty()) {
        text = QString("all");
    }

    setData(0,0,QString("%1 (%2)").arg(name).arg(text));
}

PluginItem::PluginItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent,plugin_type)
{
    plugininterface = 0;
    plugindecoderinterface = 0;
    pluginviewerinterface = 0;
    plugincontrolinterface = 0;

    widget = 0;
    dockWidget = 0;

    mode = ModeShow;
    type = 0;
}

PluginItem::~PluginItem()
{

}

void PluginItem::update()
{
    QStringList types;
    QStringList list = plugininterface->infoConfig();

    if(pluginviewerinterface)
        types << "View";
    if(plugindecoderinterface)
        types << "Decode";
    if(plugincontrolinterface)
        types << "Ctrl";

    setData(0,0,QString("%1 (%2|%3 %4)").arg(name).arg(types.join("")).arg(list.size()).arg(filename));
}

Project::Project()
{

}

Project::~Project()
{

}

void Project::Clear()
{
    ecu->clear();
    pfilter->clear();
}

bool Project::Load(QString filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
             return false;

    EcuItem *ecuitem = 0;
    ApplicationItem *applicationitem = 0;
    ContextItem *contextitem = 0;
    FilterItem *filteritem = 0;
    PluginItem *pluginitem = 0;
    PluginItem *pluginitemexist = 0;

    ecu->clear();
    pfilter->clear();
    //plugin->clear();

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
          xml.readNext();

          if(xml.isStartElement())
          {
              if(xml.name() == QString("ecu"))
              {
                  ecuitem = new EcuItem();

              }
              if(xml.name() == QString("application"))
              {
                  applicationitem = new ApplicationItem();

              }
              if(xml.name() == QString("context"))
              {
                  contextitem = new ContextItem();

              }
              if(xml.name() == QString("pfilter"))
              {
                  filteritem = new FilterItem();

              }
              if(xml.name() == QString("plugin"))
              {
                  pluginitem = new PluginItem();

              }
              if(xml.name() == QString("id"))
              {
                  if(contextitem)
                      contextitem->id = xml.readElementText();
                  else if(applicationitem)
                      applicationitem->id = xml.readElementText();
                  else if(ecuitem)
                      ecuitem->id = xml.readElementText();
              }
              if(xml.name() == QString("description"))
              {
                  if(contextitem)
                      contextitem->description = xml.readElementText();
                  else if(applicationitem)
                      applicationitem->description = xml.readElementText();
                  else if(ecuitem)
                      ecuitem->description = xml.readElementText();
              }
              if(xml.name() == QString("loglevel"))
              {
                  if(contextitem)
                      contextitem->loglevel = xml.readElementText().toInt();
                  else if(ecuitem)
                      ecuitem->loglevel = xml.readElementText().toInt();
              }
              if(xml.name() == QString("tracestatus"))
              {
                  if(contextitem)
                      contextitem->tracestatus = xml.readElementText().toInt();
                  else if(ecuitem)
                      ecuitem->tracestatus = xml.readElementText().toInt();
              }
              if(xml.name() == QString("verbosemode"))
              {
                  if(ecuitem)
                      ecuitem->verbosemode = xml.readElementText().toInt();
              }
              if(xml.name() == QString("timingpackets"))
              {
                  if(ecuitem)
                      ecuitem->timingPackets = xml.readElementText().toInt();
              }
              if(xml.name() == QString("sendgetloginfo"))
              {
                  if(ecuitem)
                      ecuitem->sendGetLogInfo = xml.readElementText().toInt();
              }
              if(xml.name() == QString("updatedata"))
              {
                  if(ecuitem)
                      ecuitem->updateDataIfOnline = xml.readElementText().toInt();
              }
              if(xml.name() == QString("interface"))
              {
                  if(ecuitem)
                      ecuitem->interfacetype = xml.readElementText().toInt();

              }
              if(xml.name() == QString("hostname"))
              {
                  if(ecuitem)
                    ecuitem->hostname = xml.readElementText();

              }
              if(xml.name() == QString("tcpport"))
              {
                  if(ecuitem)
                    ecuitem->tcpport = xml.readElementText().toInt();

              }
              if(xml.name() == QString("port"))
              {
                  if(ecuitem)
                    ecuitem->port = xml.readElementText();

              }
              if(xml.name() == QString("baudrate"))
              {
                  if(ecuitem)
                      ecuitem->baudrate = xml.readElementText().toInt();

              }
              if(xml.name() == QString("sendserialheadertcp"))
              {
                  if(ecuitem)
                      ecuitem->sendSerialHeaderTcp = xml.readElementText().toInt();

              }
              if(xml.name() == QString("sendserialheaderserial"))
              {
                  if(ecuitem)
                      ecuitem->sendSerialHeaderSerial = xml.readElementText().toInt();

              }
              if(xml.name() == QString("synctoserialheadertcp"))
              {
                  if(ecuitem)
                      ecuitem->syncSerialHeaderTcp = xml.readElementText().toInt();

              }
              if(xml.name() == QString("synctoserialheaderserial"))
              {
                  if(ecuitem)
                      ecuitem->syncSerialHeaderSerial = xml.readElementText().toInt();

              }
              if(xml.name() == QString("type"))
              {
                  if(filteritem)
                      filteritem->type = (FilterItem::FilterType)(xml.readElementText().toInt());

              }
              if(xml.name() == QString("ecuid"))
              {
                  if(filteritem)
                    filteritem->ecuId = xml.readElementText();

              }
              if(xml.name() == QString("applicationid"))
              {
                  if(filteritem)
                    filteritem->applicationId = xml.readElementText();

              }
              if(xml.name() == QString("contextid"))
              {
                  if(filteritem)
                    filteritem->contextId = xml.readElementText();

              }
              if(xml.name() == QString("headertext"))
              {
                  if(filteritem)
                    filteritem->headerText = xml.readElementText();

              }
              if(xml.name() == QString("payloadtext"))
              {
                  if(filteritem)
                    filteritem->payloadText = xml.readElementText();
              }
              if(xml.name() == QString("enableecuid"))
              {
                  if(filteritem)
                    filteritem->enableEcuId = xml.readElementText().toInt();

              }
              if(xml.name() == QString("enableapplicationid"))
              {
                  if(filteritem)
                    filteritem->enableApplicationId = xml.readElementText().toInt();;

              }
              if(xml.name() == QString("enablecontextid"))
              {
                  if(filteritem)
                    filteritem->enableContextId = xml.readElementText().toInt();;

              }
              if(xml.name() == QString("enableheadertext"))
              {
                  if(filteritem)
                    filteritem->enableHeaderText = xml.readElementText().toInt();;

              }
              if(xml.name() == QString("enablepayloadtext"))
              {
                  if(filteritem)
                    filteritem->enablePayloadText = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("enablectrlmsgs"))
              {
                  if(filteritem)
                    filteritem->enableCtrlMsgs = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("enableLogLevelMax"))
              {
                  if(filteritem)
                    filteritem->enableLogLevelMax = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("enableLogLevelMin"))
              {
                  if(filteritem)
                    filteritem->enableLogLevelMin = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("filterColour"))
              {
                  if(filteritem)
                    filteritem->filterColour = QColor(xml.readElementText());
              }
              if(xml.name() == QString("logLevelMax"))
              {
                  if(filteritem)
                    filteritem->logLevelMax = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("logLevelMin"))
              {
                  if(filteritem)
                    filteritem->logLevelMin = xml.readElementText().toInt();;
              }

              if(xml.name() == QString("name"))
              {
                  if(pluginitem)
                  {
                        QString name = xml.readElementText();
                        for(int num = 0; num < plugin->topLevelItemCount (); num++)
                        {
                            PluginItem *item = (PluginItem*)plugin->topLevelItem(num);
                            if(item->name == name)
                            {
                                pluginitemexist = item;
                            }
                        }
                  }
                  if(filteritem)
                    filteritem->name = xml.readElementText();
              }
              if(xml.name() == QString("filename"))
              {
                  if(pluginitemexist)
                    pluginitemexist->filename = xml.readElementText();

              }
              if(xml.name() == QString("mode"))
              {
                  if(pluginitemexist)
                    pluginitemexist->mode = xml.readElementText().toInt();

              }
              if(xml.name() == QString("type"))
              {
                  if(pluginitemexist)
                    pluginitemexist->type = xml.readElementText().toInt();

              }
          }
          if(xml.isEndElement())
          {
              if(xml.name() == QString("ecu"))
              {
                  if(ecu)
                  {
                    ecu->addTopLevelItem(ecuitem);
                    ecuitem->update();
                  }
                  ecuitem = 0;

              }
              if(xml.name() == QString("application"))
              {
                  if(ecuitem)
                  {
                      ecuitem->addChild(applicationitem);
                      applicationitem->update();
                  }
                  applicationitem = 0;

              }
              if(xml.name() == QString("context"))
              {
                  if(applicationitem)
                  {
                      applicationitem->addChild(contextitem);
                      contextitem->update();
                  }
                  contextitem = 0;

              }
              if(xml.name() == QString("pfilter"))
              {
                  if(pfilter)
                  {
                    pfilter->addTopLevelItem(filteritem);
                    filteritem->update();
                  }
                  filteritem = 0;

              }
              if(xml.name() == QString("plugin"))
              {
                  if(plugin)
                  {
                    delete pluginitem;
                    if(pluginitemexist)
                        pluginitemexist->update();
                  }
                  pluginitem = 0;
                  pluginitemexist = 0;

              }
          }
    }
    if (xml.hasError()) {
        QMessageBox::warning(0, QString("XML Parser error"),
                             xml.errorString());
    }

    file.close();

    return true;
}

bool Project::Save(QString filename)
{
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
             QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Cannot read project file %1:\n%2.")
                                  .arg(filename)
                                  .arg(file.errorString()));
             return false;
    }

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("dltproject");

    /* Write Configuration */
    for(int num = 0; num < ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)ecu->topLevelItem(num);
        xml.writeStartElement("ecu");

        xml.writeTextElement("id",ecuitem->id);
        xml.writeTextElement("description",ecuitem->description);
        xml.writeTextElement("interface",QString("%1").arg(ecuitem->interfacetype));
        xml.writeTextElement("hostname",ecuitem->hostname);
        xml.writeTextElement("tcpport",QString("%1").arg(ecuitem->tcpport));
        xml.writeTextElement("port",ecuitem->port);
        xml.writeTextElement("baudrate",QString("%1").arg(ecuitem->baudrate));
        xml.writeTextElement("sendserialheadertcp",QString("%1").arg(ecuitem->sendSerialHeaderTcp));
        xml.writeTextElement("sendserialheaderserial",QString("%1").arg(ecuitem->sendSerialHeaderSerial));
        xml.writeTextElement("synctoserialheadertcp",QString("%1").arg(ecuitem->syncSerialHeaderTcp));
        xml.writeTextElement("synctoserialheaderserial",QString("%1").arg(ecuitem->syncSerialHeaderSerial));
        xml.writeTextElement("loglevel",QString("%1").arg(ecuitem->loglevel));
        xml.writeTextElement("tracestatus",QString("%1").arg(ecuitem->tracestatus));
        xml.writeTextElement("verbosemode",QString("%1").arg(ecuitem->verbosemode));
        xml.writeTextElement("timingpackets",QString("%1").arg(ecuitem->timingPackets));
        xml.writeTextElement("sendgetloginfo",QString("%1").arg(ecuitem->sendGetLogInfo));
        xml.writeTextElement("updatedata",QString("%1").arg(ecuitem->updateDataIfOnline));

        for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
        {
            ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);
            xml.writeStartElement("application");

            xml.writeTextElement("id",appitem->id);
            xml.writeTextElement("description",appitem->description);

            for(int numcontext = 0; numcontext < appitem->childCount(); numcontext++)
            {
                ContextItem * conitem = (ContextItem *) appitem->child(numcontext);
                xml.writeStartElement("context");

                xml.writeTextElement("id",conitem->id);
                xml.writeTextElement("description",conitem->description);
                xml.writeTextElement("loglevel",QString("%1").arg(conitem->loglevel));
                xml.writeTextElement("tracestatus",QString("%1").arg(conitem->tracestatus));

                xml.writeEndElement(); // context
            }

            xml.writeEndElement(); // application
        }

        xml.writeEndElement(); // ecu
    }

    /* Write PFilter */
    for(int num = 0; num < pfilter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)pfilter->topLevelItem(num);
        xml.writeStartElement("pfilter");

        xml.writeTextElement("type",QString("%1").arg((int)(item->type)));

        xml.writeTextElement("name",item->name);
        xml.writeTextElement("ecuid",item->ecuId);
        xml.writeTextElement("applicationid",item->applicationId);
        xml.writeTextElement("contextid",item->contextId);
        xml.writeTextElement("headertext",item->headerText);
        xml.writeTextElement("payloadtext",item->payloadText);

        xml.writeTextElement("enableecuid",QString("%1").arg(item->enableEcuId));
        xml.writeTextElement("enableapplicationid",QString("%1").arg(item->enableApplicationId));
        xml.writeTextElement("enablecontextid",QString("%1").arg(item->enableContextId));
        xml.writeTextElement("enableheadertext",QString("%1").arg(item->enableHeaderText));
        xml.writeTextElement("enablepayloadtext",QString("%1").arg(item->enablePayloadText));
        xml.writeTextElement("enablectrlmsgs",QString("%1").arg(item->enableCtrlMsgs));
        xml.writeTextElement("enableLogLevelMin",QString("%1").arg(item->enableLogLevelMin));
        xml.writeTextElement("enableLogLevelMax",QString("%1").arg(item->enableLogLevelMax));

        xml.writeTextElement("filterColour",item->filterColour.name());

        xml.writeTextElement("logLevelMax",QString("%1").arg(item->logLevelMax));
        xml.writeTextElement("logLevelMin",QString("%1").arg(item->logLevelMin));

        xml.writeEndElement(); // filter
    }

    /* Write Plugin */
    for(int num = 0; num < plugin->topLevelItemCount (); num++)
    {
        PluginItem *item = (PluginItem*)plugin->topLevelItem(num);
        xml.writeStartElement("plugin");

        xml.writeTextElement("name",item->name);
        xml.writeTextElement("filename",item->filename);

        xml.writeTextElement("mode",QString("%1").arg(item->mode));
        xml.writeTextElement("type",QString("%1").arg(item->type));

        xml.writeEndElement(); // plugin
    }

    xml.writeEndElement(); // dltproject
    xml.writeEndDocument();

    file.close();

    return true;
}

bool Project::SaveFilter(QString filename)
{
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
            QMessageBox::critical(0, QString("DLT Viewer"),QString("Save DLT Filter file failed!"));
            return false;
    }

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("dltfilter");


    /* Write PFilter */
    for(int num = 0; num < pfilter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)pfilter->topLevelItem(num);
        xml.writeStartElement("filter");

        xml.writeTextElement("type",QString("%1").arg((int)(item->type)));

        xml.writeTextElement("name",item->name);
        xml.writeTextElement("ecuid",item->ecuId);
        xml.writeTextElement("applicationid",item->applicationId);
        xml.writeTextElement("contextid",item->contextId);
        xml.writeTextElement("headertext",item->headerText);
        xml.writeTextElement("payloadtext",item->payloadText);

        xml.writeTextElement("enableecuid",QString("%1").arg(item->enableEcuId));
        xml.writeTextElement("enableapplicationid",QString("%1").arg(item->enableApplicationId));
        xml.writeTextElement("enablecontextid",QString("%1").arg(item->enableContextId));
        xml.writeTextElement("enableheadertext",QString("%1").arg(item->enableHeaderText));
        xml.writeTextElement("enablepayloadtext",QString("%1").arg(item->enablePayloadText));
        xml.writeTextElement("enablectrlmsgs",QString("%1").arg(item->enableCtrlMsgs));
        xml.writeTextElement("enableLogLevelMin",QString("%1").arg(item->enableLogLevelMin));
        xml.writeTextElement("enableLogLevelMax",QString("%1").arg(item->enableLogLevelMax));

        xml.writeTextElement("filterColour",item->filterColour.name());

        xml.writeTextElement("logLevelMax",QString("%1").arg(item->logLevelMax));
        xml.writeTextElement("logLevelMin",QString("%1").arg(item->logLevelMin));

        xml.writeEndElement(); // filter
    }

    xml.writeEndElement(); // dltfilter
    xml.writeEndDocument();

    file.close();

    return true;
}

bool Project::LoadFilter(QString filename){

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(0, QString("DLT Viewer"),QString("Loading DLT Filter file failed!"));
        return false;
    }

    FilterItem *filteritem = 0;


    pfilter->clear();
    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
          xml.readNext();

          if(xml.isStartElement())
          {

              if(xml.name() == QString("filter"))
              {
                  filteritem = new FilterItem();

              }
              if(xml.name() == QString("type"))
              {
                  if(filteritem)
                      filteritem->type = (FilterItem::FilterType)(xml.readElementText().toInt());

              }
              if(xml.name() == QString("name"))
              {
                  if(filteritem)
                      filteritem->name = xml.readElementText();

              }
              if(xml.name() == QString("ecuid"))
              {
                  if(filteritem)
                    filteritem->ecuId = xml.readElementText();

              }
              if(xml.name() == QString("applicationid"))
              {
                  if(filteritem)
                    filteritem->applicationId = xml.readElementText();

              }
              if(xml.name() == QString("contextid"))
              {
                  if(filteritem)
                    filteritem->contextId = xml.readElementText();

              }
              if(xml.name() == QString("headertext"))
              {
                  if(filteritem)
                    filteritem->headerText = xml.readElementText();

              }
              if(xml.name() == QString("payloadtext"))
              {
                  if(filteritem)
                    filteritem->payloadText = xml.readElementText();
              }
              if(xml.name() == QString("enableecuid"))
              {
                  if(filteritem)
                    filteritem->enableEcuId = xml.readElementText().toInt();

              }
              if(xml.name() == QString("enableapplicationid"))
              {
                  if(filteritem)
                    filteritem->enableApplicationId = xml.readElementText().toInt();;

              }
              if(xml.name() == QString("enablecontextid"))
              {
                  if(filteritem)
                    filteritem->enableContextId = xml.readElementText().toInt();;

              }
              if(xml.name() == QString("enableheadertext"))
              {
                  if(filteritem)
                    filteritem->enableHeaderText = xml.readElementText().toInt();;

              }
              if(xml.name() == QString("enablepayloadtext"))
              {
                  if(filteritem)
                    filteritem->enablePayloadText = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("enablectrlmsgs"))
              {
                  if(filteritem)
                    filteritem->enableCtrlMsgs = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("enableLogLevelMax"))
              {
                  if(filteritem)
                    filteritem->enableLogLevelMax = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("enableLogLevelMin"))
              {
                  if(filteritem)
                    filteritem->enableLogLevelMin = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("filterColour"))
              {
                  if(filteritem)
                    filteritem->filterColour = QColor(xml.readElementText());
              }
              if(xml.name() == QString("logLevelMax"))
              {
                  if(filteritem)
                    filteritem->logLevelMax = xml.readElementText().toInt();;
              }
              if(xml.name() == QString("logLevelMin"))
              {
                  if(filteritem)
                    filteritem->logLevelMin = xml.readElementText().toInt();;
              }
          }
          if(xml.isEndElement())
          {
              if(xml.name() == QString("filter"))
              {
                  if(pfilter)
                  {
                    pfilter->addTopLevelItem(filteritem);
                    filteritem->update();
                  }
                  filteritem = 0;

              }

          }
    }
    if (xml.hasError()) {
        QMessageBox::warning(0, QString("XML Parser error"),
                             xml.errorString());
    }

    file.close();

    return true;
}
