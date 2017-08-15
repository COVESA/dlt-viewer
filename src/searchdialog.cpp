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
 * \file searchdialog.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "mainwindow.h"
#include "dltsettingsmanager.h"
#include "optmanager.h"

#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{

    ui->setupUi(this);
    regexpCheckBox = ui->checkBoxRegExp;
    CheckBoxSearchtoList = ui->checkBoxSearchIndex;
    match = false;
    onceClicked = false;
    startLine = -1;
    is_PayloadStartFound = false;
    is_PayloadEndFound = false;
    is_PayLoadRangeValid = false;

    lineEdits = new QList<QLineEdit*>();
    lineEdits->append(ui->lineEditText);

    bool checked = DltSettingsManager::getInstance()->value("other/search/checkBoxSearchIndex", bool(true)).toBool();
    ui->checkBoxSearchIndex->setChecked(checked);

    updateColorbutton();
}

SearchDialog::~SearchDialog()
{
    clearCacheHistory();
    delete ui;
}

void SearchDialog::setText(QString text) { ui->lineEditText->setText(text);}
void SearchDialog::selectText(){ui->lineEditText->setFocus();ui->lineEditText->selectAll();}
void SearchDialog::setHeader(bool header) { ui->checkBoxHeader->setCheckState(header?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setPayload(bool payload) { ui->checkBoxPayload->setCheckState(payload?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setCaseSensitive(bool caseSensitive) { ui->checkBoxCaseSensitive->setCheckState(caseSensitive?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setRegExp(bool regExp) { ui->checkBoxRegExp->setCheckState(regExp?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setNextClicked(bool next){nextClicked = next;}
void SearchDialog::setMatch(bool matched){match=matched;}
void SearchDialog::setStartLine(int start){startLine=start;}
void SearchDialog::setOnceClicked(bool clicked){onceClicked=clicked;}
void SearchDialog::appendLineEdit(QLineEdit *lineEdit){ lineEdits->append(lineEdit);}
void SearchDialog::setApIDText(QString text){ ui->apIdlineEdit->setText(text);}
void SearchDialog::setCtIDText(QString text){ ui->ctIdlineEdit->setText(text);}

QString SearchDialog::getText() { return ui->lineEditText->text(); }
bool SearchDialog::getHeader() { return (ui->checkBoxHeader->checkState() == Qt::Checked); }
bool SearchDialog::getPayload() { return (ui->checkBoxPayload->checkState() == Qt::Checked); }
bool SearchDialog::getCaseSensitive() { return (ui->checkBoxCaseSensitive->checkState() == Qt::Checked); }
bool SearchDialog::getRegExp() { return (ui->checkBoxRegExp->checkState() == Qt::Checked); }
bool SearchDialog::getSearchFromBeginning(){return (ui->radioButtonPosBeginning->isChecked());}
bool SearchDialog::getNextClicked(){return nextClicked;}
bool SearchDialog::getMatch(){return match;}
bool SearchDialog::getOnceClicked(){return onceClicked;}
bool SearchDialog::searchtoIndex(){return (ui->checkBoxSearchIndex->checkState() ==Qt::Checked);}

int SearchDialog::getStartLine( ){return startLine;}
QString SearchDialog::getApIDText(){ return ui->apIdlineEdit->text();}
QString SearchDialog::getCtIDText(){ return ui->ctIdlineEdit->text();}
QString SearchDialog::getTimeStampStart(){return ui->timeStartlineEdit->text();}
QString SearchDialog::getTimeStampEnd(){return ui->timeEndlineEdit->text();}
QString SearchDialog::getPayLoadStampStart(){return ui->payloadStartlineEdit->text();}
QString SearchDialog::getPayLoadStampEnd(){return ui->payloadEndlineEdit->text();}

void SearchDialog::setSearchColour(QLineEdit *lineEdit,int result)
{
    QPalette palette = lineEdit->palette();
    QColor text0(255,255,255);
    QColor text1(0,0,0);
    QColor background0(255,102,102);
    QColor background1(255,255,255);
    switch(result){
    case 0:
        palette.setColor(QPalette::Text,text0);
        lineEdit->setPalette(palette);
        palette.setColor(QPalette::Base,background0);
        lineEdit->setPalette(palette);
        break;
    case 1:
        palette.setColor(QPalette::Text,text1);
        lineEdit->setPalette(palette);
        palette.setColor(QPalette::Base,background1);
        lineEdit->setPalette(palette);
        break;
    }
}

void SearchDialog::focusRow(int searchLine)
{
    TableModel *model = qobject_cast<TableModel *>(table->model());
    QModelIndex idx = model->index(searchLine, 0, QModelIndex());
    table->scrollTo(idx, QAbstractItemView::EnsureVisible);
    model->setLastSearchIndex(searchLine);
    table->selectionModel()->clear();
    model->modelChanged();
}

int SearchDialog::find()
{
    emit addActionHistory();
    QRegExp searchTextRegExp;

    int searchLine;
    int searchBorder;

    emit searchProgressChanged(true);

    if(file->sizeFilter()==0) {
        emit searchProgressChanged(false);
        return 0;
    }

    if( (getMatch() || getSearchFromBeginning()==false) && false == searchtoIndex() ){
        QModelIndexList list = table->selectionModel()->selection().indexes();
        if(list.count() > 0)
        {
            QModelIndex index;
            for(int num=0; num < list.count();num++)
            {
                index = list[num];
                if(index.column()==0)
                {
                    break;
                }
            }
            setStartLine(index.row());
        }
    }

    searchLine = getStartLine();
    searchBorder = getStartLine();;
    if(searchBorder < 0 || searchtoIndex()){
        if(getNextClicked() || searchtoIndex()){
            searchBorder = file->sizeFilter()==0?0:file->sizeFilter()-1;
        }else{
            searchBorder = 0;
        }

    }

    if(getRegExp())
    {
        searchTextRegExp.setPattern(getText());
        searchTextRegExp.setCaseSensitivity(getCaseSensitive()?Qt::CaseSensitive:Qt::CaseInsensitive);
        if (!searchTextRegExp.isValid())
        {
            QMessageBox::warning(0, QString("Search"),
                                 QString("Invalid regular expression!"));
            emit searchProgressChanged(false);
            return 0;
        }
    }

    findProcess(searchLine,searchBorder,searchTextRegExp,getApIDText(),getCtIDText(),getTimeStampStart(), getTimeStampEnd(),getPayLoadStampStart(),getPayLoadStampEnd());

    emit searchProgressChanged(false);

    if (searchtoIndex())
    {
        cacheSearchHistory();
        emit refreshedSearchIndex();
        //if at least one element has been found -> successful search
        if ( 0 < m_searchtablemodel->get_SearchResultListSize())
            return 1;
    }

    if(getMatch())
    {
        return 1;
    }
    setStartLine(0);
    return 0;
}


void SearchDialog::findProcess(int searchLine, int searchBorder, QRegExp &searchTextRegExp,QString apID, QString ctID, QString tStart, QString tEnd, QString tpayloadStart, QString tpayloadEnd)
{

    QDltMsg msg;
    QByteArray buf;
    QString text;
    QString headerText;

    int ctr = 0;
    Qt::CaseSensitivity is_Case_Sensitive = Qt::CaseInsensitive;

    if(getCaseSensitive())
    {
        is_Case_Sensitive = Qt::CaseSensitive;
    }

    payloadStart = tpayloadStart;
    payloadEnd = tpayloadEnd;
    is_payLoadSearchSelected = false;
    is_TimeStampSearchSelected = false;

    tempPayLoad.clear();

    if((false == payloadStart.isEmpty()) && ( false == payloadEnd.isEmpty()))
    {
        is_payLoadSearchSelected = true;
    }

    if((tStart.size()) && (tEnd.size()))
    {
        is_TimeStampSearchSelected = true;
    }

    is_PayloadStartFound = false;
    is_PayloadEndFound = false;
    is_PayLoadRangeValid = false;

    m_searchtablemodel->clear_SearchResults();

    QProgressDialog fileprogress("Searching...", "Abort", 0, file->sizeFilter(), this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::NonModal);
    fileprogress.show();

    bool silentMode = !OptManager::getInstance()->issilentMode();

    do
    {
        ctr++;

        text.clear();

        if(getNextClicked() || searchtoIndex()){
            searchLine++;
            if(searchLine >= file->sizeFilter()){
                searchLine = 0;
            }
        }
        else
        {
            searchLine--;
            if(searchLine <= -1)
            {
                searchLine = file->sizeFilter()-1;
            }
        }

        /* Update progress every 0.5% */
        if(searchLine%1000==0)
        {

            fileprogress.setValue(ctr);

            if(fileprogress.wasCanceled())
            {
                break;
            }
            QApplication::processEvents();
        }

        /* get the message with the selected item id */
        buf = file->getMsgFilter(searchLine);
        msg.setMsg(buf);
        if(DltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool())
            pluginManager->decodeMsg(msg,silentMode);

        bool pluginFound = false;
        headerText.clear();
        /* search header */
        if(!pluginFound || text.isEmpty())
        {
            text += msg.toStringHeader();
            tempPayLoad = msg.toStringPayload();

        }
        headerText = text;
        /*Assuming that the timeStamp is the 3rd value always*/
        QString timeSt = headerText.section(" ",2,2);


        is_TimeStampRangeValid = false;

        if(true == is_TimeStampSearchSelected)
        {
            if((tStart.toFloat() <= timeSt.toFloat()))
            {
                if((tEnd.toFloat() >= timeSt.toFloat()))
                {
                    is_TimeStampRangeValid = true;
                }
            }
        }

        payLoadValidityCheck();

        if(getHeader())
        {
            if (getRegExp())
            {
                if(text.contains(searchTextRegExp))
                {
                    if ( foundLine(searchLine) )
                        break;
                    else
                        continue;
                }else {
                    setMatch(false);
                }
            }
            else
            {
                if(getText().isEmpty())
                {
                    if(apID.size())
                    {
                        if(ctID.size())
                        {
                            if(headerText.contains(apID,is_Case_Sensitive) && headerText.contains(ctID,is_Case_Sensitive))
                            {
                                if(timeStampPayloadValidityCheck(searchLine))
                                    break;
                                else
                                    continue;
                            }
                        }
                        else
                        {
                            if(headerText.contains(apID,is_Case_Sensitive))
                            {
                                if(timeStampPayloadValidityCheck(searchLine))
                                    break;
                                else
                                    continue;
                            }
                        }
                    }
                    else if(ctID.size())
                    {
                        if(headerText.contains(ctID,is_Case_Sensitive))
                        {
                            if(timeStampPayloadValidityCheck(searchLine))
                                break;
                            else
                                continue;
                        }
                    }
                    else if ((is_TimeStampSearchSelected) || (is_payLoadSearchSelected))
                    {
                        if(timeStampPayloadValidityCheck(searchLine))
                            break;
                        else
                            continue;
                    }
                    else
                    {
                        if ( foundLine(searchLine) )
                            break;
                        else
                            continue;
                    }
                }
                else if(headerText.contains(getText(),is_Case_Sensitive))
                {
                    if(apID.size())
                    {
                        if(ctID.size())
                        {
                            if(headerText.contains(apID,is_Case_Sensitive) && headerText.contains(ctID,is_Case_Sensitive))
                            {
                                if(timeStampPayloadValidityCheck(searchLine))
                                    break;
                                else
                                    continue;
                            }
                        }
                        else
                        {
                            if(headerText.contains(apID,is_Case_Sensitive))
                            {
                                if(timeStampPayloadValidityCheck(searchLine))
                                    break;
                                else
                                    continue;
                            }
                        }
                    }
                    else if(ctID.size())
                    {
                        if(headerText.contains(ctID,is_Case_Sensitive))
                        {
                            if(timeStampPayloadValidityCheck(searchLine))
                                break;
                            else
                                continue;
                        }
                    }
                    else if ((is_TimeStampSearchSelected) || (is_payLoadSearchSelected))
                    {
                        if(timeStampPayloadValidityCheck(searchLine))
                            break;
                        else
                            continue;
                    }
                }
                else
                {
                    setMatch(false);
                }
            }
        }

        /* search payload */
        text.clear();
        if(!pluginFound || text.isEmpty())
        {
            text += msg.toStringPayload();
        }

        if(getPayload())
        {
            if (getRegExp())
            {
                if(tempPayLoad.contains(searchTextRegExp))
                {
                    if ( foundLine(searchLine) )
                        break;
                    else
                        continue;
                }else {
                    setMatch(false);
                }
            }
            else
            {
                if(getText().isEmpty())
                {
                    if(apID.size())
                    {
                        if(ctID.size())
                        {
                            if(headerText.contains(apID,is_Case_Sensitive) && headerText.contains(ctID,is_Case_Sensitive))
                            {
                                if(timeStampPayloadValidityCheck(searchLine))
                                {
                                    break;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            if(headerText.contains(apID,is_Case_Sensitive))
                            {
                                if(timeStampPayloadValidityCheck(searchLine))
                                {
                                    break;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                        }
                    }
                    else if(ctID.size())
                    {
                        if(headerText.contains(ctID,is_Case_Sensitive))
                        {
                            if(timeStampPayloadValidityCheck(searchLine))
                            {
                                break;
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }
                    else
                    {
                        if(timeStampPayloadValidityCheck(searchLine))
                        {
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
                else if(tempPayLoad.contains(getText(),is_Case_Sensitive))
                {
                    if(apID.size())
                    {
                        if(ctID.size())
                        {
                            if(headerText.contains(apID,is_Case_Sensitive) && headerText.contains(ctID,is_Case_Sensitive))
                            {
                                if(timeStampPayloadValidityCheck(searchLine))
                                {
                                    break;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            if(headerText.contains(apID,is_Case_Sensitive))
                            {
                                if(timeStampPayloadValidityCheck(searchLine))
                                {
                                    break;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                        }
                    }
                    else if(ctID.size())
                    {
                        if(headerText.contains(ctID,is_Case_Sensitive))
                        {
                            if(timeStampPayloadValidityCheck(searchLine))
                            {
                                break;
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }
                    else
                    {
                        if(timeStampPayloadValidityCheck(searchLine))
                        {
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
                else
                {
                    setMatch(false);
                }
            }
        }
    }while( searchBorder != searchLine );

}

bool SearchDialog::payLoadValidityCheck()
{
    // When the start payload is found, consider range as valid
    if((tempPayLoad.contains(payloadStart)) && (false == is_PayloadEndFound))
    {
        is_PayLoadRangeValid = true;
        is_PayloadStartFound = true;
    }

    if(true == is_PayloadStartFound)
    {
        if(tempPayLoad.contains(payloadEnd))
        {

            is_PayloadEndFound = true;
            is_PayLoadRangeValid = false;
        }
    }

    return is_PayLoadRangeValid;
}

bool SearchDialog::timeStampPayloadValidityCheck(int searchLine)
{
    if(is_TimeStampSearchSelected)
    {
        if(is_TimeStampRangeValid)
        {
            if(is_payLoadSearchSelected)
            {
                if(is_PayLoadRangeValid)
                    if(foundLine(searchLine))
                        return true;
            }
            else
            {
                if(foundLine(searchLine))
                    return true;
            }
        }
    }
    else if(is_payLoadSearchSelected)
    {
        if(is_PayLoadRangeValid)
            if(foundLine(searchLine))
                return true;
    }
    else
    {
        if(foundLine(searchLine))
            return true;
    }
    return false;
}

bool SearchDialog::timeStampCheck(int searchLine)
{
    bool retVal = false;
    // If time stamps are given, they are considered to be of highest priority
    if(is_TimeStampSearchSelected)
    {
        if(is_TimeStampRangeValid)
        {
            if(is_payLoadSearchSelected)
            {
                if(payLoadValidityCheck())
                {
                    if(foundLine(searchLine))
                    {
                        retVal = true;
                    }
                }
            }
            else
            {
                if(foundLine(searchLine))
                {
                    retVal = true;
                }
                else
                {
                    retVal = false;
                }
            }
        }
        else
        {
            retVal = false;
        }
    }
    else
    {
        if(is_payLoadSearchSelected)
        {
            if(payLoadValidityCheck())
            {
                if(foundLine(searchLine))
                    return true;
            }
        }
        else if(foundLine(searchLine))
            return true;
        else
            return false;
    }
    return retVal;
}

bool SearchDialog::foundLine(int searchLine)
{
    setMatch(true);

    if (searchtoIndex())
    {
        addToSearchIndex(searchLine);
        emit refreshedSearchIndex();
    }
    else
    {
        focusRow(searchLine);
        setStartLine(searchLine);
        return true;//found single result, and breaking here
    }
    return false;//don't break search here
}

void SearchDialog::on_pushButtonNext_clicked()
{
    /* For every new search, start payLoad and EndpayLoad will be different and hence member variable storing previous found
     * value shoudl eb reset. */
    is_PayloadStartFound = false;
    is_PayloadEndFound = false;
    is_PayLoadRangeValid = false;

    setNextClicked(true);
    int result = find();
    for(int i=0; i<lineEdits->size();i++){
       setSearchColour(lineEdits->at(i),result);
    }
}

void SearchDialog::on_pushButtonPrevious_clicked()
{
    setNextClicked(false);
    int result = find();
    for(int i=0; i<lineEdits->size();i++){
       setSearchColour(lineEdits->at(i),result);
    }
}
void SearchDialog::findNextClicked(){
    setNextClicked(true);
    int result = find();
    for(int i=0; i<lineEdits->size();i++){
       setSearchColour(lineEdits->at(i),result);
    }
}
void SearchDialog::findPreviousClicked(){
    setNextClicked(false);
    int result = find();
    for(int i=0; i<lineEdits->size();i++){
       setSearchColour(lineEdits->at(i),result);
    }
}

void SearchDialog::on_lineEditText_textEdited(QString newText)
{
        lineEdits->at(1)->setText(newText);
        for(int i=0; i<lineEdits->size();i++){
            if(lineEdits->at(0)->text().isEmpty())
                setSearchColour(lineEdits->at(i),1);
        }
}
void SearchDialog::textEditedFromToolbar(QString newText){
        lineEdits->at(0)->setText(newText);
        for(int i=0; i<lineEdits->size();i++){
            if(lineEdits->at(0)->text().isEmpty())
                setSearchColour(lineEdits->at(i),1);
        }
}

void SearchDialog::on_pushButtonColor_clicked()
{
    QString color = DltSettingsManager::getInstance()->value("other/searchResultColor", QString("#00AAFF")).toString();
    QColor oldColor(color);
    QColor newColor = QColorDialog::getColor(oldColor, this, "Pick color for Search Highlight");
    if(!newColor.isValid())
    {
        // User cancelled
        return;
    }

    DltSettingsManager::getInstance()->setValue("other/searchResultColor", newColor.name());
    updateColorbutton();
}

void SearchDialog::updateColorbutton()
{
    QString color = DltSettingsManager::getInstance()->value("other/searchResultColor", QString("#00AAFF")).toString();
    QColor hlColor(color);
    QPixmap px(12, 12);
    px.fill(hlColor);
    ui->pushButtonColor->setIcon(px);
}


void SearchDialog::addToSearchIndex(int searchLine)
{
    m_searchtablemodel->add_SearchResultEntry(file->getMsgFilterPos(searchLine));    

 }

void SearchDialog::registerSearchTableModel(SearchTableModel *model)
{
    m_searchtablemodel = model;    
}



void SearchDialog::on_checkBoxSearchIndex_toggled(bool checked)
{
    DltSettingsManager::getInstance()->setValue("other/search/checkBoxSearchIndex", checked);
}

void SearchDialog::loadSearchHistory()
{
    // getting text of the action button clicked to load search history.
    QAction *action = qobject_cast<QAction *>(sender());
    QString text;
    if(action)
    {
        text = action->text();
    }

    // creating a local list to store the indexes related to the key retrieved from the cache.
    QList <unsigned long> tmp ;
    if(cachedHistoryKey.size() > 0)
    {
        tmp = cachedHistoryKey[text];

        //deleting the previous search list and adding the cached search obtained to the model.
        m_searchtablemodel->clear_SearchResults();
        for (int i = 0;i < tmp.size();i++)
        {
            m_searchtablemodel->add_SearchResultEntry(tmp.at(i));
        }
    }
    emit refreshedSearchIndex();
}

void SearchDialog::cacheSearchHistory()
{
    // if it is a new search then add all the indexes of the search to a list(m_searchHistory).
    QString searchBoxText = getText();  
    m_searchHistory.append(m_searchtablemodel->m_searchResultList);
    cachedHistoryKey.insert(searchBoxText,m_searchHistory.last());    
}

void SearchDialog::clearCacheHistory()
{
    // obtaining the list of keys stored in cache
    cachedHistoryKey.clear();
}
