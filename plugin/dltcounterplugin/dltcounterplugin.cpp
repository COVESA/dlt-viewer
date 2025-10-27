/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file dummycontrolplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>
#include <QTextStream>
#include <QTableView>
#include <QAbstractItemModel>
#include <QMessageBox>

#include "dltcounterplugin.h"

DltCounterPlugin::DltCounterPlugin()
{
    form = NULL;
    counterMessages = 0;
    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;
    dltFile = 0;
    dltControl = 0;
    ecuList = 0;

    loadingCompleteTimer = new QTimer(this);
    loadingCompleteTimer->setSingleShot(true);
    connect(loadingCompleteTimer, &QTimer::timeout, this, &DltCounterPlugin::dataConsolidatedMap);
}

DltCounterPlugin::~DltCounterPlugin()
{

}

QString DltCounterPlugin::name()
{
    return QString("DLT Counter Plugin");
}

QString DltCounterPlugin::pluginVersion(){
    return DLT_COUNTER_PLUGIN_VERSION;
}

QString DltCounterPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DltCounterPlugin::description()
{
    return QString();
}

QString DltCounterPlugin::error()
{
    return QString();
}

bool DltCounterPlugin::loadConfig(QString /*filename */ )
{
    return true;
}

bool DltCounterPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DltCounterPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DltCounterPlugin::initViewer()
{
    form = new DltCounter::Form(this);
    return form;
}

bool DltCounterPlugin::initControl(QDltControl *control)
{
    dltControl = control;

    return true;
}

bool DltCounterPlugin::initConnections(QStringList list)
{
    ecuList = new QStringList(list);
    // form->setConnections(list);

    return false;
}

bool DltCounterPlugin::controlMsg(int , QDltMsg &)
{
    return false;
}

void DltCounterPlugin::updateCounters(int ,int )
{

}

bool DltCounterPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname){
    Q_UNUSED(index)
    Q_UNUSED(connectionState)
    Q_UNUSED(hostname)

#if QT_5_SUPPORTED_VERSION
    //qDebug() << ecuList->at(index) << "ConnectionState:" << connectionState << "Hostname:" << hostname << Qt::endl;
#else
    //qDebug() << ecuList->at(index) << "ConnectionState:" << connectionState << "Hostname:" << hostname << endl;
#endif
    return true;
}

bool DltCounterPlugin::autoscrollStateChanged(bool enabled)
{
    // form->autoscrollStateChanged(enabled);

    return true;
}

void DltCounterPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
    Q_UNUSED(pMessageDecoder);
}

// The tableview has been parsed to another global variable to access for pre-indexing
void DltCounterPlugin::initMainTableView(QTableView* pTableView)
{
    // Q_UNUSED(pTableView);
    if (!pTableView) {
        qWarning() << "Main TableView pointer is null!";
        return;
    }

    m_mainTableView = pTableView;

}

// The index has been collected in the QHash map in prior for faster scroll back
// to the mainwindow to find the previous or next message to the missing message.
void DltCounterPlugin::buildTableIndex()
{
    if (!m_mainTableView || !m_mainTableView->model()) return;

    QAbstractItemModel *model = m_mainTableView->model();
    const int ctidColumn = 6;
    const int counterColumn = 3;

    ctidCounterRowMap.clear();

    for (int row = 0; row < model->rowCount(); ++row) {
        QString ctid = model->index(row, ctidColumn).data().toString();
        int counter = model->index(row, counterColumn).data().toInt();
        ctidCounterRowMap[ctid][counter] = row;
    }
}

// The function will scroll back in the mainWindow to either previous or next message
// The function access the tableview as model and then match to the context id and counter
// value received. It utilises the QHash map which has been filled in buildTableIndex
void DltCounterPlugin::scrollToCounterInMainTable(const QString &ctid, int counter){

    if (!m_mainTableView) return;

    QAbstractItemModel* model = m_mainTableView->model();
    if(!model) return;

    const QHash<int, int> &counterMap = ctidCounterRowMap.value(ctid);
    int row = counterMap.value(counter);
    QModelIndex index = model->index(row, 0);
    if (!index.isValid()) {
        QMessageBox::warning(nullptr, "Invalid Index", "Index is not valid.");
        return;
    }

    m_mainTableView->scrollTo(index, QAbstractItemView::PositionAtCenter);
    m_mainTableView->clearSelection();
    m_mainTableView->selectRow(row);
    m_mainTableView->setFocus();
}

void DltCounterPlugin::configurationChanged()
{}

void DltCounterPlugin::selectedIdxMsg(int , QDltMsg &) {

}

void DltCounterPlugin::selectedIdxMsgDecoded(int , QDltMsg &){

}

void DltCounterPlugin::initFileStart(QDltFile *file){

    clearAll();
    dltFile = file;
}

// The counter data and the context IDs in the log file are accessed
// The data is saved to QList
void DltCounterPlugin::initMsg(int, QDltMsg &msg){

    QString name = msg.getCtid();
    nameList.append(name);

    unsigned char count = msg.getMessageCounter();
    countList.append(count);
}

// The data which are separated as QList, it will be merged to a QMap.
// The consolidatedMap will be accessed by counter push button and export push button.
void DltCounterPlugin::dataConsolidatedMap() {

    for (int i = 0; i < nameList.size() && i < countList.size(); ++i) {
        QString name = nameList[i];
        int count = static_cast<int>(countList[i]);

        consolidatedMap[name].insert(count);
    }
}

void DltCounterPlugin::initMsgDecoded(int , QDltMsg &){

}

//Consolidated data and pre collection of index occurs after the file has been
// completed finished initialising.
void DltCounterPlugin::initFileFinish(){
    dataConsolidatedMap();
    buildTableIndex();
}

void DltCounterPlugin::updateFileStart(){

}

void DltCounterPlugin::updateMsg(int , QDltMsg &){

}

void DltCounterPlugin::updateMsgDecoded(int , QDltMsg &){

}

//Consolidated data and pre collection of index occurs after the file has been
// completed finished initialising.
void DltCounterPlugin::updateFileFinish(){
    dataConsolidatedMap();
    buildTableIndex();

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(dummycontrolplugin, DummyControlPlugin);
#endif
