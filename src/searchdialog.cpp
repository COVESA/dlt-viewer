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
 * \file searchdialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "mainwindow.h"
#include "qdltoptmanager.h"

#include <dltmessagematcher.h>

#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSignalBlocker>


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

    lineEdits = new QList<QLineEdit*>();
    lineEdits->append(ui->lineEditText);
    table = nullptr;

    // at start we want to know if single step search or "fill search table mode" is active !
    bool checked = QDltSettingsManager::getInstance()->value("other/search/checkBoxSearchIndex", bool(true)).toBool();
    ui->checkBoxSearchIndex->setChecked(checked);

    checked = QDltSettingsManager::getInstance()->value("other/search/checkBoxHeader", bool(true)).toBool();
    ui->checkBoxHeader->setChecked(checked);

    checked = QDltSettingsManager::getInstance()->value("other/search/checkBoxCasesensitive", bool(true)).toBool();
    ui->checkBoxCaseSensitive->setChecked(checked);

    checked = QDltSettingsManager::getInstance()->value("other/search/checkBoxRegEx", bool(true)).toBool();
    ui->checkBoxRegExp->setChecked(checked);

    fSilentMode = !QDltOptManager::getInstance()->issilentMode();

    updateColorbutton();
}

SearchDialog::~SearchDialog()
{
    clearCacheHistory();
    delete ui;
}

void SearchDialog::selectText(){ui->lineEditText->setFocus();ui->lineEditText->selectAll();}
void SearchDialog::setHeader(bool header) { ui->checkBoxHeader->setCheckState(header?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setPayload(bool payload) { ui->checkBoxPayload->setCheckState(payload?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setCaseSensitive(bool caseSensitive) { ui->checkBoxCaseSensitive->setCheckState(caseSensitive?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setRegExp(bool regExp) { ui->checkBoxRegExp->setCheckState(regExp?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setNextClicked(bool next){nextClicked = next;}
void SearchDialog::setMatch(bool matched){match=matched;}


void SearchDialog::setOnceClicked(bool clicked){onceClicked=clicked;}
void SearchDialog::appendLineEdit(QLineEdit *lineEdit){ lineEdits->append(lineEdit);}

QString SearchDialog::getText() { return ui->lineEditText->text(); }

void SearchDialog::abortSearch()
{
    isSearchCancelled = true;
}

bool SearchDialog::getHeader()
{
    return (ui->checkBoxHeader->checkState() == Qt::Checked);
}

bool SearchDialog::getPayload()
{
    return (ui->checkBoxPayload->checkState() == Qt::Checked);
}

bool SearchDialog::getRegExp()
{
    return (ui->checkBoxRegExp->checkState() == Qt::Checked);
}

bool SearchDialog::getNextClicked(){return nextClicked;}
bool SearchDialog::getOnceClicked(){return onceClicked;}

QString SearchDialog::getApIDText(){ return ui->apIdlineEdit->text();}
QString SearchDialog::getCtIDText(){ return ui->ctIdlineEdit->text();}

QString SearchDialog::getTimeStampStart()
{
    //qDebug() << "content of start time" << ui->timeStartlineEdit->text()<< __LINE__;
    return ui->timeStartlineEdit->text();
}

QString SearchDialog::getTimeStampEnd()
{
    //qDebug() << "content of end time" << ui->timeEndlineEdit->text() << __LINE__;
    return ui->timeEndlineEdit->text();
}

bool SearchDialog::getCaseSensitive()
{
    //qDebug() << "getCaseSensitive is" << ui->checkBoxCaseSensitive->checkState() << __LINE__;
    return (ui->checkBoxCaseSensitive->checkState() == Qt::Checked);
}

bool SearchDialog::searchtoIndex()
{
    //qDebug() << "searchtoIndex is" << ui->checkBoxSearchIndex->checkState() << __LINE__;
    return (ui->checkBoxSearchIndex->checkState() == Qt::Checked);
}


bool SearchDialog::getSearchFromBeginning()
{
    return (ui->radioButtonPosBeginning->isChecked());
}

void SearchDialog::setStartLine(long int start)
{
  startLine=start;
}


void SearchDialog::setSearchColour(QLineEdit *lineEdit,int result)
{
    QPalette palette = lineEdit->palette();
    QColor text0 = QColor(255,255,255);
    QColor text1 = QColor(0,0,0);
    QColor background0 = QColor(255,102,102);
    QColor background1 = QColor(255,255,255);

    if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
    {
        background1 = QColor(31,31,31);
        text1 = QColor(255,255,255);
    }

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

void SearchDialog::focusRow(long int searchLine)
{
    TableModel *model = qobject_cast<TableModel *>(table->model());
    QModelIndex idx = model->index(searchLine, 0, QModelIndex());
    //qDebug() << "Focus row in message table window" << searchLine << __FILE__ << __LINE__;

    table->scrollTo(idx, QAbstractItemView::EnsureVisible);
    table->scrollTo(idx, QAbstractItemView::PositionAtCenter);

    model->setMarker(searchLine, highlightColor);

    model->setLastSearchIndex(searchLine);
    table->selectionModel()->clear();
    model->modelChanged();
}

int SearchDialog::find()
{
    isSearchCancelled = false;

    emit addActionHistory();
    QRegularExpression searchTextRegExpression;
    is_TimeStampSearchSelected = false;
    long int searchBorder;
    long int lStartLine;

    emit searchProgressChanged(true);

    if(file->sizeFilter()==0)
    {
        emit searchProgressChanged(false);
        return 0;
    }

   if( ( (match == true) || ( getSearchFromBeginning() == false )) && false == searchtoIndex() )
    {
        // single step search
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
   else
   {
      focusRow(-1);
   }


    if ( true == getSearchFromBeginning() )
    {
      //qDebug() << "Start from the beginning" << __LINE__;
    }
    else
    {
        if (table->selectionModel() != nullptr )
         {
          {
           if ( false == table->selectionModel()->selectedIndexes().isEmpty() )
            {
             if (table->selectionModel()->selectedIndexes().first().row() > -1)
              {
               lStartLine = table->selectionModel()->selectedIndexes().first().row();
               setStartLine( lStartLine );
              }
           }
          }
         }
         qDebug() << "Search starting at line" << startLine;
    }

    searchBorder = startLine;
    if(searchBorder < 0 || searchtoIndex())
    {
        if(getNextClicked() || searchtoIndex())
        {
            searchBorder = file->sizeFilter()==0?0:file->sizeFilter()-1;
        }
        else
        {
            searchBorder = 0;
        }

    }

    if(getRegExp() == true)
    {
        searchTextRegExpression.setPattern(getText());
        if (searchTextRegExpression.isValid() == false)
        {
            if ( false == fSilentMode)
            {
            QMessageBox::warning(0, QString("Search"), QString("Invalid regular expression!"));
            }
            emit searchProgressChanged(false);
            return 1;
        }

        int options = QRegularExpression::DotMatchesEverythingOption;
        if (!getCaseSensitive())
            options |= QRegularExpression::CaseInsensitiveOption;
        searchTextRegExpression.setPatternOptions(static_cast<QRegularExpression::PatternOption>(options));
    }

    //check timestamp search pattern
    TimeStampStarttime = getTimeStampStart();
    TimeStampStoptime = getTimeStampEnd();

    if( ( TimeStampStarttime.size() > 0 ) && ( TimeStampStoptime.size() > 0) )
    {
        dTimeStampStart = TimeStampStarttime.toDouble();
        dTimeStampStop = TimeStampStoptime.toDouble();
        if( (dTimeStampStop -  dTimeStampStart) >= 0 )
         {
         //qDebug() << "Timestamp search enabled" << dTimeStampStart << dTimeStampStop << __LINE__;
         is_TimeStampSearchSelected = true;
         }
        else
        {
         qDebug() << "Invalid timestamp range" << dTimeStampStart << dTimeStampStop << __LINE__;
         is_TimeStampSearchSelected = false;
         if ( false == fSilentMode)
         {
         QMessageBox::warning(0, QString("Search"), QString("Invalid timestamp range !"));
         }
         emit searchProgressChanged(false);
         return 1;
        }
    }

    //check APID and CTID search
    stApid = getApIDText();
    stCtid = getCtIDText();
    if( stApid.size() > 0 || stCtid.size() > 0 ) // so we need to consider what is given here
    {
        if( stApid.size() > 4 || stCtid.size() > 4 )
        {
            qDebug() << "Given APID or CTID exceeds limit !";
            if ( false == fSilentMode)
            {
            QMessageBox::warning(0, QString("Search"), QString("Given APID or CTID exceeds limit !"));
            }
            emit searchProgressChanged(false);
            return 2;
        }
        fIs_APID_CTID_requested = true;
    }
    else
    {
     fIs_APID_CTID_requested = false;
    }

    findMessages(startLine,searchBorder,searchTextRegExpression);

    emit searchProgressChanged(false);

    if (searchtoIndex() == true )
    {
        cacheSearchHistory();
        emit refreshedSearchIndex();
        //if at least one element has been found -> successful search
        if ( 0 < m_searchtablemodel->get_SearchResultListSize())
        {
            return 1;
        }
    }

    if(match == true )
    {
        return 1;
    }
    setStartLine(-1); // so we do not miss index 0 any longer ...
    return 0;
}


void SearchDialog::findMessages(long int searchLine, long int searchBorder, QRegularExpression &searchTextRegExp)
{

    QDltMsg msg;
    QByteArray buf;
    int ctr = 0;
    Qt::CaseSensitivity is_Case_Sensitive = Qt::CaseInsensitive;

    starttime();

    if(getCaseSensitive() == true)
    {
        is_Case_Sensitive = Qt::CaseSensitive;
    }

    m_searchtablemodel->clear_SearchResults();

    bool msgIdEnabled=QDltSettingsManager::getInstance()->value("startup/showMsgId", true).toBool();
    QString msgIdFormat=QDltSettingsManager::getInstance()->value("startup/msgIdFormat", "0x%x").toString();

    DltMessageMatcher matcher;
    matcher.setCaseSentivity(is_Case_Sensitive);
    matcher.setSearchAppId(stApid);
    matcher.setSearchCtxId(stCtid);
    if (is_TimeStampSearchSelected) {
        matcher.setTimestapmRange(dTimeStampStart, dTimeStampStop);
    }
    if (msgIdEnabled) {
        matcher.setMessageIdFormat(msgIdFormat);
    }
    matcher.setHeaderSearchEnabled(getHeader());
    matcher.setPayloadSearchEnabled(getPayload());

    do
    {
        ctr++; // for file progress indication

        if(getNextClicked() || searchtoIndex())
        {
            searchLine++;
            if(searchLine >= file->sizeFilter())
            {
                searchLine = 0;
            }
        }
        else // go back
        {
            searchLine--;
            if(searchLine <= -1)
            {
                searchLine = file->sizeFilter()-1;
            }
        }

        // Update progress every 0.5%
        if(searchLine%1000 == 0)
        {
            QApplication::processEvents();
            if (isSearchCancelled) {
                break;
            }
            emit searchProgressValueChanged(static_cast<int>(ctr * 100.0 / file->sizeFilter()));
        }

        /* get the message with the selected item id */
        buf = file->getMsgFilter(searchLine);
        msg.setMsg(buf);
        msg.setIndex(file->getMsgFilterPos(searchLine));

        /* decode the message if desired - could this call be avoided as the message is already decoded elsewhere ? */
        if(QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool())
        {
            //qDebug() << "Decode" << __LINE__;
            pluginManager->decodeMsg(msg, fSilentMode);
        }

        const bool matchFound = getRegExp() ? matcher.match(msg, searchTextRegExp) : matcher.match(msg, getText());
        if (!matchFound)
        {
            match = false;
            continue;
        }

        if (foundLine(searchLine))
            break;
        else
            continue;
    }
    while( searchBorder != searchLine );
    stoptime();
}

bool SearchDialog::foundLine(long int searchLine)
{
    match = true;

    if (searchtoIndex() == true)
    {
        addToSearchIndex(searchLine);
        emit refreshedSearchIndex();
    }
    else
    {
        focusRow(searchLine); // focus the line ein message table view
        setStartLine(searchLine);
        //qDebug() << "Single line hit in  " << searchLine << __LINE__;
        return true;//found single result, and breaking here
    }
    return false;//don't break search here
}

void SearchDialog::on_pushButtonNext_clicked() // connected to main window line 424
{
    /* For every new search, start payLoad and EndpayLoad will be different and hence member variable storing previous found
     * value shoudl eb reset. */
    setNextClicked(true);
    int result = find();
    for(int i=0; i<lineEdits->size();i++)
    {
       setSearchColour(lineEdits->at(i),result);
    }
}

void SearchDialog::on_pushButtonPrevious_clicked()
{
    setNextClicked(false);
    int result = find();
    for(int i=0; i<lineEdits->size();i++)
    {
       setSearchColour(lineEdits->at(i),result);
    }
}

void SearchDialog::findNextClicked()
{
    setNextClicked(true);

    int result = find();
    for(int i=0; i<lineEdits->size();i++)
    {
       setSearchColour(lineEdits->at(i),result);
    }
}

void SearchDialog::findPreviousClicked()
{
    setNextClicked(false);

    int result = find();
    for(int i=0; i<lineEdits->size();i++){
       setSearchColour(lineEdits->at(i),result);
    }
}

void SearchDialog::on_lineEditText_textEdited(QString newText)
{
        {
            // block signal so that it does not trigger a setText back on lineEdits->at(0)!
            QSignalBlocker signalBlocker(lineEdits->at(1));
            lineEdits->at(1)->setText(newText);
        }
        for(int i=0; i<lineEdits->size();i++){
            if(lineEdits->at(0)->text().isEmpty())
                setSearchColour(lineEdits->at(i),1);
        }
}
void SearchDialog::textEditedFromToolbar(QString newText)
{
        {
            // block signal so that it does not trigger a setText back on lineEdits->at(1)!
            QSignalBlocker signalBlocker(lineEdits->at(0));
            lineEdits->at(0)->setText(newText);
        }
        for(int i=0; i<lineEdits->size();i++){
            if(lineEdits->at(0)->text().isEmpty())
                setSearchColour(lineEdits->at(i),1);
        }
}

void SearchDialog::on_pushButtonColor_clicked()
{
    QString color = QDltSettingsManager::getInstance()->value("other/searchResultColor", QString("#00AAFF")).toString();
    QColor oldColor(color);
    QColor newColor = QColorDialog::getColor(oldColor, this, "Pick color for Search Highlight");
    if(false == newColor.isValid())
    {
        // User cancelled
        return;
    }

    QDltSettingsManager::getInstance()->setValue("other/searchResultColor", newColor.name());
    updateColorbutton();
}

void SearchDialog::updateColorbutton()
{
    QString color = QDltSettingsManager::getInstance()->value("other/searchResultColor", QString("#00AAFF")).toString();
    QColor lhlColor(color);
    highlightColor = lhlColor;
    QPixmap px(12, 12);
    px.fill(highlightColor);
    ui->pushButtonColor->setIcon(px);
}


void SearchDialog::addToSearchIndex(long int searchLine)
{
    //qDebug() << "Add hit line to search table" << searchLine << __LINE__;
    m_searchtablemodel->add_SearchResultEntry(file->getMsgFilterPos(searchLine));    
 }

void SearchDialog::registerSearchTableModel(SearchTableModel *model)
{
    m_searchtablemodel = model;    
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

void SearchDialog::saveSearchHistory(QStringList& searchHistory) {
    //To save the search history
    QSettings settings("MyApp", "SearchHistory");
    settings.beginWriteArray("history");
    int count = qMin(searchHistory.size(), 20);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue("entry", searchHistory.at(i));
    }
    settings.endArray();
}

void SearchDialog::loadSearchHistoryList(QStringList& searchHistory)
{
  //To retrive the search history once DLT Viewer restarts
    QSettings settings("MyApp", "SearchHistory");
    searchHistory.clear();
    int size = settings.beginReadArray("history");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        searchHistory.append(settings.value("entry").toString());
    }
    settings.endArray();
}

void SearchDialog::on_checkBoxHeader_toggled(bool checked)
{
   QDltSettingsManager::getInstance()->setValue("other/search/checkBoxHeader", checked);
}


void SearchDialog::on_checkBoxSearchIndex_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxSearchIndex", checked);
    setStartLine(-1);
}

void SearchDialog::on_checkBoxCaseSensitive_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxCasesensitive", checked);
}

void SearchDialog::on_checkBoxRegExp_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxRegEx", checked);
}


void SearchDialog::starttime(void)
{
long int temps;

#if defined(_MSC_VER)
   SYSTEMTIME systemtime;
   GetSystemTime(&systemtime);
   time_t timestamp_sec;
   time(&timestamp_sec);
   temps = (time_t)timestamp_sec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    temps = (time_t)tv.tv_sec;
#endif
    searchseconds = temps;

}

void SearchDialog::stoptime(void)
{
long int temps;
long int dtemps;

#if defined(_MSC_VER)
   SYSTEMTIME systemtime;
   GetSystemTime(&systemtime);
   time_t timestamp_sec;
   time(&timestamp_sec);
   temps = (time_t)timestamp_sec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    temps = (time_t)tv.tv_sec;
#endif

    dtemps = temps - searchseconds;
    qDebug() << "Time for search [s]" << dtemps;
}


