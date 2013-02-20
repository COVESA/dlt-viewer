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
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{

    ui->setupUi(this);
    regexpCheckBox = ui->checkBoxRegExp;
    match = false;
    onceClicked = false;
    startLine = -1;

    lineEdits = new QList<QLineEdit*>();
    lineEdits->append(ui->lineEditText);
}

SearchDialog::~SearchDialog()
{
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

QString SearchDialog::getText() { return ui->lineEditText->text(); }
bool SearchDialog::getHeader() { return (ui->checkBoxHeader->checkState() == Qt::Checked); }
bool SearchDialog::getPayload() { return (ui->checkBoxPayload->checkState() == Qt::Checked); }
bool SearchDialog::getCaseSensitive() { return (ui->checkBoxCaseSensitive->checkState() == Qt::Checked); }
bool SearchDialog::getRegExp() { return (ui->checkBoxRegExp->checkState() == Qt::Checked); }
bool SearchDialog::getSearchFromBeginning(){return (ui->radioButtonPosBeginning->isChecked());}
bool SearchDialog::getNextClicked(){return nextClicked;}
bool SearchDialog::getMatch(){return match;}
bool SearchDialog::getOnceClicked(){return onceClicked;}
int SearchDialog::getStartLine( ){return startLine;}

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

int SearchDialog::find()
{
    QRegExp searchTextRegExp;
    QDltMsg msg;
    QByteArray buf;
    QString text;
    int searchLine;
    int searchBorder;


    if(file->sizeFilter()==0)
            return 0;

    //setSearchColour(QColor(0,0,0),QColor(255,255,255));

    if(getMatch() || getSearchFromBeginning()==false){
        QModelIndexList list = table->selectionModel()->selection().indexes();
        if(list.count()<=0)
        {
            QMessageBox::critical(0, QString("DLT Viewer"),QString("No message selected"));
            setMatch(false);
            //table->clearSelection();
            //setSearchColour(QColor(255,255,255),QColor(255,102,102));
            return 0;
        }

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

    searchLine = getStartLine();
    searchBorder = getStartLine();;
    if(searchBorder < 0){
        if(getNextClicked()){
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
            //setSearchColour(QColor(255,255,255),QColor(255,102,102));
            return 0;
        }
    }


    QProgressDialog fileprogress("Searching...", "Abort", 0, file->sizeFilter(), this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);
    fileprogress.show();
    do
    {

        if(getNextClicked()){
            searchLine++;
            if(searchLine >= file->sizeFilter()){
                searchLine = 0;
                //QMessageBox::information(0, QString("Search"),QString("End of file reached. Search start from top."));
            }
        }else{
            searchLine--;
            if(searchLine <= -1){
                searchLine = file->sizeFilter()-1;
                //QMessageBox::information(0, QString("Search"),QString("Top of file reached. Search start from bottom."));
            }
        }

        //qDebug()<<"startLine: "<<getStartLine();
        //qDebug()<<"searchBorder: "<<searchBorder;
        //qDebug()<<"searchLine: "<<searchLine;

        if(getNextClicked()){
            fileprogress.setValue(searchLine+1);
        }else{
            fileprogress.setValue(file->sizeFilter()-searchLine);
        }

        /* get the message with the selected item id */
        buf = file->getMsgFilter(searchLine);
        msg.setMsg(buf);
        for(int num2 = 0; num2 < plugin->topLevelItemCount (); num2++)
        {
            PluginItem *item = (PluginItem*)plugin->topLevelItem(num2);

            if(item->getMode() != item->ModeDisable && item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg,1))
            {
                item->plugindecoderinterface->decodeMsg(msg,1);
                break;
            }
        }

        bool pluginFound = false;

        /* search header */
        if(!pluginFound || text.isEmpty())
        {
            text += msg.toStringHeader();
        }

        if(getHeader())
        {
            if (getRegExp())
            {
                if(text.contains(searchTextRegExp))
                {
                    table->selectRow(searchLine);
                    setStartLine(searchLine);
                    setMatch(true);
                    break;
                }else {
                    setMatch(false);
                }
            }
            else
            {
                if(text.contains(getText(),getCaseSensitive()? Qt::CaseSensitive : Qt::CaseInsensitive ))
                {

                    table->selectRow(searchLine);
                    setStartLine(searchLine);
                    setMatch(true);
                    break;
                }else {
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
                if(text.contains(searchTextRegExp))
                {
                    table->selectRow(searchLine);
                    setMatch(true);
                    setStartLine(searchLine);
                    break;
                }else {
                    setMatch(false);
                }
            }
            else
            {
                if(text.contains(getText(),getCaseSensitive()?Qt::CaseSensitive:Qt::CaseInsensitive))
                {
                    table->selectRow(searchLine);
                    setMatch(true);
                    setStartLine(searchLine);
                    break;
                } else {
                    setMatch(false);
                }
            }
        }

    }while(searchBorder != searchLine);

    if(getMatch())
    {
        return 1;
    }

    //table->clearSelection();
    //setSearchColour(QColor(255,255,255),QColor(255,102,102));
    return 0;
}

void SearchDialog::on_pushButtonNext_clicked()
{
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
