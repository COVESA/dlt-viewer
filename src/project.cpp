/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file project.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include <QHeaderView>

#include "project.h"
#include "dltsettingsmanager.h"
#include "dltuiutils.h"
#include "dlt_user.h"

const char *loginfo[] = {"default","off","fatal","error","warn","info","debug","verbose","","","","","","","","",""};
const char *traceinfo[] = {"default","off","on"};

EcuItem::EcuItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent,ecu_type)
{
    /* initialise receive buffer and message*/
    id = "ECU";
    description = "A new ECU";
    interfacetype = 0; /* default TCP */
    hostname = "localhost";
    tcpport = DLT_DAEMON_TCP_PORT;
    baudrate = BAUD115200; /* default 115200 */
    loglevel = DLT_LOG_INFO;
    tracestatus = DLT_TRACE_STATUS_OFF;
    verbosemode = 1;
    sendSerialHeaderTcp = false;
    sendSerialHeaderSerial = true;
    syncSerialHeaderTcp = false;
    syncSerialHeaderSerial = true;
    timingPackets = false;
    sendGetLogInfo = false;
    sendDefaultLogLevel = true;
    sendGetSoftwareVersion = true;
    updateDataIfOnline = true;
    autoReconnect = true;
    autoReconnectTimeout = 5;

    totalBytesRcvd = 0;
    totalBytesRcvdLastTimeout = 0;

    tryToConnect = 0;
    connected = 0;

    status = EcuItem::unknown;

    m_serialport = 0;

    /* Limit size of socket receiption buffer to limit application buffer size */
    /* qt sets buffer normally to unlimited */
    //socket.setReadBufferSize(64000);

    autoReconnectTimestamp = QDateTime::currentDateTime();
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

bool EcuItem::operator< ( const QTreeWidgetItem & other ) const {

    int column = treeWidget()->header()->sortIndicatorSection();

    int fieldWidth = 0;

    if(column==0){
        fieldWidth=4;
    }

    QString currentItem = QString("%1").arg(text(column),fieldWidth,QLatin1Char('0'));
    QString otherItem = QString("%1").arg(other.text(column),fieldWidth,QLatin1Char('0'));

//    qDebug()<<"currentItemEcu: "<<currentItem <<" otherItemEcu: "<<otherItem;

    return currentItem.toLower() < otherItem.toLower();
}

bool EcuItem::isAutoReconnectTimeoutPassed(){

    bool timeoutPassed = false;

    QDateTime currentDateTime = QDateTime::currentDateTime();

    //qDebug() << "currentDateTime:" << currentDateTime << " - nextTimeOut:" << autoReconnectTimestamp;

    if(autoReconnectTimestamp <= currentDateTime){
        timeoutPassed = true;
        autoReconnectTimestamp = currentDateTime.addSecs(autoReconnectTimeout);
    }

    return timeoutPassed;
}

void EcuItem::updateAutoReconnectTimestamp(){

    autoReconnectTimestamp = QDateTime::currentDateTime().addSecs(autoReconnectTimeout);
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

bool ApplicationItem::operator< ( const QTreeWidgetItem & other ) const {

    int column = treeWidget()->header()->sortIndicatorSection();

    int fieldWidth = 0;

    if(column==0){
        fieldWidth=4;
    }

    QString currentItem = QString("%1").arg(text(column),fieldWidth,QLatin1Char('0'));
    QString otherItem = QString("%1").arg(other.text(column),fieldWidth,QLatin1Char('0'));

//    qDebug()<<"currentItemApp: "<<currentItem <<" otherItemApp: "<<otherItem;

    return currentItem.toLower() < otherItem.toLower();
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


bool ContextItem::operator< ( const QTreeWidgetItem & other ) const {

    int column = treeWidget()->header()->sortIndicatorSection();

    int fieldWidth = 0;

    if(column==0){
        fieldWidth=4;
    }

    QString currentItem = QString("%1").arg(text(column),fieldWidth,QLatin1Char('0'));
    QString otherItem = QString("%1").arg(other.text(column),fieldWidth,QLatin1Char('0'));

//    qDebug()<<"currentItemCt: "<<currentItem <<" otherItemCt: "<<otherItem;

    return currentItem.toLower() < otherItem.toLower();
}


FilterItem::FilterItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent,filter_type)
{
    filter.type = QDltFilter::positive;

    filter.name = "New Filter";

    setCheckState(0,Qt::Checked);
    filter.enableRegexp_Context = false;
    filter.enableRegexp_Header = false;
    filter.enableRegexp_Payload = false;
    filter.enableFilter = false;
    filter.enableEcuid = false;
    filter.enableApid = false;
    filter.enableCtid = false;
    filter.enableHeader = false;
    filter.enablePayload = false;
    filter.enableLogLevelMax = false;
    filter.enableLogLevelMin = false;
    filter.enableCtrlMsgs = false;
    filter.enableMarker = false;

    filter.filterColour = QColor();

    filter.logLevelMax = 6;
    filter.logLevelMin = 0;

    /* Prevent drag&drop childify */
    setFlags(flags() ^ Qt::ItemIsDropEnabled);
}

FilterItem::~FilterItem()
{

}

void FilterItem:: operator = (FilterItem &item)
{
    filter = item.filter;

}


void FilterItem::update()
{
    QString text;

    switch(filter.type)
    {
    case QDltFilter::positive:
        if(filter.isMarker())
            text += QString("POSITIVE MARKER ");
        else
            text += QString("POSITIVE ");
        break;
    case QDltFilter::negative:
        if(filter.isMarker())
            text += QString("NEGATIVE MARKER ");
        else
            text += QString("NEGATIVE ");
        break;
    case QDltFilter::marker:
        text += QString("MARKER ");
        break;
    }

    if(filter.enableRegexp_Context || filter.enableRegexp_Header || filter.enableRegexp_Payload){
        text += "RegExp";
    }

    if(filter.enableFilter){
        setCheckState(0,Qt::Checked);
    }else{
        setCheckState(0,Qt::Unchecked);
    }

    if(filter.enableEcuid ) {
        text += QString("%1 ").arg(filter.ecuid);
    }
    if(filter.enableApid ) {
        text += QString("%1 ").arg(filter.apid);
    }
    if(filter.enableCtid ) {
        text += QString("%1 ").arg(filter.ctid);
    }
    if(filter.enableHeader ) {
        text += QString("%1 ").arg(filter.header);
    }
    if(filter.enablePayload ) {
        text += QString("%1 ").arg(filter.payload);
    }
    if(filter.enableCtrlMsgs ) {
        text += QString("CtrlMsgs ");
    }
    if(filter.enableLogLevelMax ) {
        switch(filter.logLevelMax)
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
    if(filter.enableLogLevelMin ) {
        switch(filter.logLevelMin)
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
    if(filter.isMarker())
    {
        text += filter.filterColour.name();

        setBackground(0,filter.filterColour);
        setBackground(1,filter.filterColour);
        setForeground(0,DltUiUtils::optimalTextColor(filter.filterColour));
        setForeground(1,DltUiUtils::optimalTextColor(filter.filterColour));
    }

    if(text.isEmpty()) {
        text = QString("all");
    }

    setData(1,0,QString("%1 (%2)").arg(filter.name).arg(text));
}


MyPluginDockWidget::MyPluginDockWidget(){
    pluginitem = NULL;
}

MyPluginDockWidget::MyPluginDockWidget(PluginItem *i, QWidget *parent){

        this->pluginitem = i;

        QDockWidget(this->pluginitem->getName(),parent);

        this->setWindowTitle(this->pluginitem->getName());

}

MyPluginDockWidget::~MyPluginDockWidget(){

}


void MyPluginDockWidget::closeEvent(QCloseEvent *event)
{
    pluginitem->setMode(QDltPlugin::ModeDisable);

    pluginitem->update();

    pluginitem->savePluginModeToSettings();

    QDockWidget::closeEvent(event);
}



PluginItem::PluginItem(QTreeWidgetItem *parent, QDltPlugin *_plugin)
    : QTreeWidgetItem(parent,plugin_type)
{

    widget = 0;
    dockWidget = 0;

    loader = 0;

    mode = QDltPlugin::ModeShow;
    type = 0;

    plugin = _plugin;
}

PluginItem::~PluginItem()
{
    if(loader != NULL)
    {
        loader->unload();
        delete loader;
    }
}

void PluginItem::update()
{
    QStringList types;
    QStringList list = plugin->infoConfig();

    if(plugin->isViewer())
        types << "View";
    if(plugin->isDecoder())
        types << "Decode";
    if(plugin->isControl())
        types << "Ctrl";
    if(plugin->isCommand())
        types << "Command";

    QString *modeString;
    switch(plugin->getMode()){
        case 0:
            modeString = new QString("Disabled");
            break;
        case 1:
            modeString = new QString("Enabled&Not visible");
            break;
        case 2:
            modeString = new QString("Enabled&Visible");
            break;
        default:
            modeString = new QString("");
            break;
    }

    //setData(0,0,QString("%1 (%2:%3|%4 %5)").arg(this->getName()).arg(types.join("")).arg(*modeString).arg(list.size()).arg(this->getFilename()));
    setData(0,0,QString("%1").arg(plugin->getName()));
    setData(1,0,QString("%1").arg(types.join("")));
    setData(2,0,QString("%1").arg(*modeString));
    setData(3,0,QString("%1").arg(list.size()));
    setData(4,0,QString("%1").arg(this->getFilename()));

    delete modeString;
}

QString PluginItem::getName(){
    return plugin->getName();
}

QString PluginItem::getPluginVersion(){
    return plugin->getPluginVersion();
}

QString PluginItem::getPluginInterfaceVersion(){
    return plugin->getPluginInterfaceVersion();
}

QString PluginItem::getFilename(){
    return filename;
}
void PluginItem::setFilename(QString f){
    filename = f;
}

int PluginItem::getType(){
    return type;
}
void PluginItem::setType(int t){
    type = t;
}

int PluginItem::getMode(){
    return plugin->getMode();
}

void PluginItem::setMode(int t)
{
    plugin->setMode((QDltPlugin::Mode)t);
}

void PluginItem::savePluginModeToSettings(){
    DltSettingsManager::getInstance()->setValue("plugin/pluginmodefor"+this->getName(),QVariant(plugin->getMode()));
}

void PluginItem::loadPluginModeFromSettings(){
    plugin->setMode((QDltPlugin::Mode)DltSettingsManager::getInstance()->value("plugin/pluginmodefor"+this->getName(),QVariant(QDltPlugin::ModeDisable)).toInt());
}

Project::Project()
{
    ecu = NULL;
    filter = NULL;
    plugin = NULL;
    settings = NULL;
}

Project::~Project()
{

}

void Project::Clear()
{
    ecu->clear();
    filter->clear();
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
    filter->clear();
    //plugin->clear();

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
          xml.readNext();

          if(xml.isStartElement())
          {
              /* Project settings */
              if(xml.name() == QString("autoConnect"))
              {
                  settings->autoConnect = xml.readElementText().toInt();
              }
              if(xml.name() == QString("autoScroll"))
              {
                  settings->autoScroll = xml.readElementText().toInt();
              }
              if(xml.name() == QString("autoMarkFatalError"))
              {
                  settings->autoMarkFatalError = xml.readElementText().toInt();
              }
              if(xml.name() == QString("autoMarkWarn"))
              {
                  settings->autoMarkWarn = xml.readElementText().toInt();
              }
              if(xml.name() == QString("autoMarkMarker"))
              {
                  settings->autoMarkMarker = xml.readElementText().toInt();
              }
              if(xml.name() == QString("fontSize"))
              {
                  settings->fontSize = xml.readElementText().toInt();
              }
              if(xml.name() == QString("automaticTimeSettings"))
              {
                  settings->automaticTimeSettings = xml.readElementText().toInt();
              }
              if(xml.name() == QString("utcOffset"))
              {
                  settings->utcOffset = xml.readElementText().toLongLong();
              }
              if(xml.name() == QString("utcOffset"))
              {
                  settings->dst = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showIndex"))
              {
                  settings->showIndex = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showTime"))
              {
                  settings->showTime = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showTimestamp"))
              {
                  settings->showTimestamp = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showCount"))
              {
                  settings->showCount = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showEcuId"))
              {
                  settings->showEcuId = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showApId"))
              {
                  settings->showApId = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showApIdDesc"))
              {
                  settings->showApIdDesc = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showCtId"))
              {
                  settings->showCtId = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showCtIdDesc"))
              {
                  settings->showCtIdDesc = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showSessionId"))
              {
                  settings->showSessionId = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showSessionName"))
              {
                  settings->showSessionName = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showType"))
              {
                  settings->showType = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showSubtype"))
              {
                  settings->showSubtype = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showMode"))
              {
                  settings->showMode = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showNoar"))
              {
                  settings->showNoar = xml.readElementText().toInt();
              }
              if(xml.name() == QString("showPayload"))
              {
                  settings->showPayload = xml.readElementText().toInt();
              }
              if(xml.name() == QString("loggingOnlyMode"))
              {
                  settings->loggingOnlyMode = xml.readElementText().toInt();
              }


              /* Connection, plugin and filter */
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
              if(xml.name() == QString("pfilter")) // this should be filter, but to be compatible keep it
              {
                  filteritem = new FilterItem();

              }
              if(xml.name() == QString("plugin"))
              {
                  pluginitem = new PluginItem(0,0);

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
              if(xml.name() == QString("sendDefaultLogLevel"))
              {
                  if(ecuitem)
                      ecuitem->sendDefaultLogLevel = xml.readElementText().toInt();
              }
              if(xml.name() == QString("sendGetSoftwareVersion"))
              {
                  if(ecuitem)
                      ecuitem->sendGetSoftwareVersion = xml.readElementText().toInt();
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
                    ecuitem->setHostname(xml.readElementText());

              }
              if(xml.name() == QString("tcpport"))
              {
                  if(ecuitem)
                    ecuitem->setTcpport(xml.readElementText().toInt());

              }
              if(xml.name() == QString("port"))
              {
                  if(ecuitem)
                    ecuitem->setPort(xml.readElementText());

              }
              if(xml.name() == QString("baudrate"))
              {
                  //TODO (BaudRateType)?
                  if(ecuitem)
                      ecuitem->setBaudrate((BaudRateType)xml.readElementText().toInt());

              }
              if(xml.name() == QString("sendserialheadertcp"))
              {
                  if(ecuitem)
                      ecuitem->setSendSerialHeaderTcp(xml.readElementText().toInt());

              }
              if(xml.name() == QString("sendserialheaderserial"))
              {
                  if(ecuitem)
                      ecuitem->setSendSerialHeaderSerial(xml.readElementText().toInt());

              }
              if(xml.name() == QString("synctoserialheadertcp"))
              {
                  if(ecuitem)
                      ecuitem->setSyncSerialHeaderTcp(xml.readElementText().toInt());

              }
              if(xml.name() == QString("synctoserialheaderserial"))
              {
                  if(ecuitem)
                      ecuitem->setSyncSerialHeaderSerial(xml.readElementText().toInt());

              }
              if(xml.name() == QString("autoReconnect"))
              {
                  if(ecuitem)
                      ecuitem->autoReconnect = xml.readElementText().toInt();

              }
              if(xml.name() == QString("autoReconnectTimeout"))
              {
                  if(ecuitem)
                      ecuitem->autoReconnectTimeout = xml.readElementText().toInt();

              }

              if(filteritem)
                filteritem->filter.LoadFilterItem(xml);

              if(xml.name() == QString("name"))
              {
                  if(pluginitem)
                  {
                        QString name = xml.readElementText();
                        for(int num = 0; num < plugin->topLevelItemCount (); num++)
                        {
                            PluginItem *item = (PluginItem*)plugin->topLevelItem(num);
                            if(item->getName() == name)
                            {
                                pluginitemexist = item;
                            }
                        }
                  }
              }
              if(xml.name() == QString("filename"))
              {
                  if(pluginitemexist)
                    pluginitemexist->setFilename( xml.readElementText() );

              }
              if(xml.name() == QString("mode"))
              {
                  if(pluginitemexist)
                    pluginitemexist->setMode( xml.readElementText().toInt() );

              }
              if(xml.name() == QString("type"))
              {
                  if(pluginitemexist)
                    pluginitemexist->setType(xml.readElementText().toInt() );

              }
          }
          if(xml.isEndElement())
          {
              if(xml.name() == QString("ecu"))
              {
                  if(ecu && ecuitem)
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
              if(xml.name() == QString("pfilter")) // this should be filter, but to be compatible keep it
              {
                  if(filter && filteritem)
                  {
                    filter->addTopLevelItem(filteritem);
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

    /* Write project settings */
    xml.writeStartElement("settings");
        xml.writeStartElement("table");
            xml.writeTextElement("fontSize",QString("%1").arg(settings->fontSize));
            xml.writeTextElement("automaticTimeSettings",QString("%1").arg(settings->automaticTimeSettings));
            xml.writeTextElement("utcOffset",QString("%1").arg(settings->utcOffset));
            xml.writeTextElement("dst",QString("%1").arg(settings->dst));
            xml.writeTextElement("showIndex",QString("%1").arg(settings->showIndex));
            xml.writeTextElement("showTime",QString("%1").arg(settings->showTime));
            xml.writeTextElement("showTimestamp",QString("%1").arg(settings->showTimestamp));
            xml.writeTextElement("showCount",QString("%1").arg(settings->showCount));
            xml.writeTextElement("showEcuId",QString("%1").arg(settings->showEcuId));
            xml.writeTextElement("showApId",QString("%1").arg(settings->showApId));
            xml.writeTextElement("showApIdDesc",QString("%1").arg(settings->showApIdDesc));
            xml.writeTextElement("showCtId",QString("%1").arg(settings->showCtId));
            xml.writeTextElement("showCtIdDesc",QString("%1").arg(settings->showCtIdDesc));
            xml.writeTextElement("showType",QString("%1").arg(settings->showType));
            xml.writeTextElement("showSubtype",QString("%1").arg(settings->showSubtype));
            xml.writeTextElement("showMode",QString("%1").arg(settings->showMode));
            xml.writeTextElement("showNoar",QString("%1").arg(settings->showNoar));
            xml.writeTextElement("showPayload",QString("%1").arg(settings->showPayload));
        xml.writeEndElement(); // table

        xml.writeStartElement("other");
            xml.writeTextElement("autoConnect",QString("%1").arg(settings->autoConnect));
            xml.writeTextElement("autoScroll",QString("%1").arg(settings->autoScroll));
            xml.writeTextElement("autoMarkFatalError",QString("%1").arg(settings->autoMarkFatalError));
            xml.writeTextElement("autoMarkWarn",QString("%1").arg(settings->autoMarkWarn));
            xml.writeTextElement("autoMarkMarker",QString("%1").arg(settings->autoMarkMarker));
            xml.writeTextElement("writeControl",QString("%1").arg(settings->writeControl));
            xml.writeTextElement("updateContextLoadingFile",QString("%1").arg(settings->updateContextLoadingFile));
            xml.writeTextElement("loggingOnlyMode",QString("%1").arg(settings->loggingOnlyMode));
        xml.writeEndElement(); // other
    xml.writeEndElement(); // settings


    /* Write Configuration */
    for(int num = 0; num < ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)ecu->topLevelItem(num);
        xml.writeStartElement("ecu");

        xml.writeTextElement("id",ecuitem->id);
        xml.writeTextElement("description",ecuitem->description);
        xml.writeTextElement("interface",QString("%1").arg(ecuitem->interfacetype));
        xml.writeTextElement("hostname",ecuitem->getHostname());
        xml.writeTextElement("tcpport",QString("%1").arg(ecuitem->getTcpport()));
        xml.writeTextElement("port",ecuitem->getPort());
        xml.writeTextElement("baudrate",QString("%1").arg(ecuitem->getBaudrate()));
        xml.writeTextElement("sendserialheadertcp",QString("%1").arg(ecuitem->getSendSerialHeaderTcp()));
        xml.writeTextElement("sendserialheaderserial",QString("%1").arg(ecuitem->getSendSerialHeaderSerial()));
        xml.writeTextElement("synctoserialheadertcp",QString("%1").arg(ecuitem->getSyncSerialHeaderTcp()));
        xml.writeTextElement("synctoserialheaderserial",QString("%1").arg(ecuitem->getSyncSerialHeaderSerial()));
        xml.writeTextElement("loglevel",QString("%1").arg(ecuitem->loglevel));
        xml.writeTextElement("tracestatus",QString("%1").arg(ecuitem->tracestatus));
        xml.writeTextElement("verbosemode",QString("%1").arg(ecuitem->verbosemode));
        xml.writeTextElement("timingpackets",QString("%1").arg(ecuitem->timingPackets));
        xml.writeTextElement("sendgetloginfo",QString("%1").arg(ecuitem->sendGetLogInfo));
        xml.writeTextElement("sendDefaultLogLevel",QString("%1").arg(ecuitem->sendDefaultLogLevel));
        xml.writeTextElement("sendGetSoftwareVersion",QString("%1").arg(ecuitem->sendGetSoftwareVersion));
        xml.writeTextElement("updatedata",QString("%1").arg(ecuitem->updateDataIfOnline));
        xml.writeTextElement("autoReconnect",QString("%1").arg(ecuitem->autoReconnect));
        xml.writeTextElement("autoReconnectTimeout",QString("%1").arg(ecuitem->autoReconnectTimeout));

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

    /* Write Filter */
    for(int num = 0; num < filter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)filter->topLevelItem(num);
        xml.writeStartElement("pfilter"); // this should be filter, but to be compatible keep it

        item->filter.SaveFilterItem(xml);

        xml.writeEndElement(); // filter
    }

    /* Write Plugin */
    for(int num = 0; num < plugin->topLevelItemCount (); num++)
    {
        PluginItem *item = (PluginItem*)plugin->topLevelItem(num);
        xml.writeStartElement("plugin");

        xml.writeTextElement("name",item->getName());
        xml.writeTextElement("filename",item->getFilename());

        xml.writeTextElement("mode",QString("%1").arg(item->getMode()));
        xml.writeTextElement("type",QString("%1").arg(item->getType()));

        xml.writeEndElement(); // plugin
    }

    xml.writeEndElement(); // dltproject
    xml.writeEndDocument();

    file.close();

    return true;
}

bool Project::SaveFilter(QString filename)
{
    QDltFilterList filterList;

    for(int num = 0; num < filter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)filter->topLevelItem(num);
        QDltFilter *filter = new QDltFilter();
        *filter = item->filter;
        filterList.filters.append(filter);
    }

    return filterList.SaveFilter(filename);
}

bool Project::LoadFilter(QString filename, bool replace){

    QDltFilterList filterList;

    filterList.LoadFilter(filename,replace);

    if(replace)
        filter->clear();

    for(int num=0;num<filterList.filters.size();num++)
    {
        FilterItem *filteritem = new FilterItem();
        filteritem->filter = *(filterList.filters[num]);
        filter->addTopLevelItem(filteritem);
        filteritem->update();
    }

    return true;

}
