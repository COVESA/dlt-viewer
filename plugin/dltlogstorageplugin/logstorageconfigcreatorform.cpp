/**
 * @licence app begin@
 * Copyright (C) 2015 Advanced Driver Information Technology
 *
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 *
 * \file logstorageconfigcreatorform.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "logstorageconfigcreatorform.h"
#include "ui_logstorageconfigcreatorform.h"

#include <QRegularExpression>
#include <QValidator>

LogstorageConfigCreatorForm::LogstorageConfigCreatorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogstorageConfigCreatorForm),
    currentFilter(0)
{
    ui->setupUi(this);

    // predefine FILTER0 as 1st entry of filter combobox
    ui->comboBox_filter->addItem("FILTER" + QString::number(currentFilter));

    // fill combo box with log levels
    QString log_levels = "DLT_LOG_VERBOSE,DLT_LOG_DEBUG,DLT_LOG_INFO,DLT_LOG_WARN,DLT_LOG_ERROR,DLT_LOG_FATAL";
    QStringList list = log_levels.split(",");

    foreach(QString item, list)
        ui->comboBox_level->addItem(item);

    // define some basic lineEdit validators
#ifdef QT5_QT6_COMPAT
    ui->lineEdit_apid->setValidator(new QRegularExpressionValidator( QRegularExpression("[a-zA-Z0-9_,]{0,20}$|[.]{1}[*]{1}"), 0));
    ui->lineEdit_ctid->setValidator(new QRegularExpressionValidator( QRegularExpression("[a-zA-Z0-9_,]{0,20}$|[.]{1}[*]{1}"), 0));
    ui->lineEdit_fname->setValidator(new QRegularExpressionValidator( QRegularExpression("[a-zA-Z0-9_-]\\S{0,20}$"), 0));
    ui->lineEdit_fsize->setValidator( new QRegularExpressionValidator( QRegularExpression("[1-9]\\d{0,7}$"), 0));
    ui->lineEdit_nofiles->setValidator( new QRegularExpressionValidator( QRegularExpression("[1-9]\\d{0,2}$"), 0));
#else
    ui->lineEdit_apid->setValidator(new QRegularExpressionValidator( QRegularExpression("[a-zA-Z0-9_,]{0,20}$|[.]{1}[*]{1}"), 0));
    ui->lineEdit_ctid->setValidator(new QRegularExpressionValidator( QRegularExpression("[a-zA-Z0-9_,]{0,20}$|[.]{1}[*]{1}"), 0));
    ui->lineEdit_fname->setValidator(new QRegularExpressionValidator( QRegularExpression("[a-zA-Z0-9_-]\\S{0,20}$"), 0));
    ui->lineEdit_fsize->setValidator( new QRegularExpressionValidator( QRegularExpression("[1-9]\\d{0,7}$"), 0));
    ui->lineEdit_nofiles->setValidator( new QRegularExpressionValidator( QRegularExpression("[1-9]\\d{0,2}$"), 0));
#endif
    setFilterDefaults();

    // create list of filters
    filters = new QHash<QString, LogstorageFilter>();

    // connect some ui actions with functionality
    connect(ui->pushButton_New,SIGNAL(clicked()), SLOT(set_button_text_Update_to_Add()));
    connect(ui->comboBox_filter,SIGNAL(currentIndexChanged(int)), SLOT(set_button_text_Add_to_Update(int)));
    connect(ui->comboBox_filter, SIGNAL(currentIndexChanged(int)),SLOT(load_filter(int)));
}

LogstorageConfigCreatorForm::~LogstorageConfigCreatorForm()
{
    // delete filters list
    if (filters != 0) {
        filters->clear();
        delete filters;
    }


    delete ui;
}

void LogstorageConfigCreatorForm::set_button_text_Add_to_Update(int idx)
{
    (void) idx; /* not used */
    ui->pushButton_Add->setText(QString("Update"));
}

void LogstorageConfigCreatorForm::set_button_text_Update_to_Add()
{
    ui->pushButton_Add->setText(QString("Add"));
}

// private QFiles
void LogstorageConfigCreatorForm::on_pushButton_New_clicked()
{
    if (validateFilter()) {
        currentFilter++;
        QString s;
        s.append("FILTER");
        s.append(QString::number(currentFilter));
        ui->comboBox_filter->addItem(s);

        int index = ui->comboBox_filter->findText(s);
        if ( index != -1 ) // -1 for not found
            ui->comboBox_filter->setCurrentIndex(index);

        setFilterDefaults();
    }
}

void LogstorageConfigCreatorForm::remove_filter(bool update)
{
    // remove filter config from text view / storage file
    ui->textEdit->moveCursor(QTextCursor::Start);
    ui->textEdit->find(ui->comboBox_filter->currentText());
    ui->textEdit->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    ui->textEdit->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
    ui->textEdit->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
    ui->textEdit->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
    ui->textEdit->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
    ui->textEdit->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
    ui->textEdit->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
    ui->textEdit->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);

    // delete text
    ui->textEdit->textCursor().removeSelectedText();

    // delete first empty line
    if (ui->comboBox_filter->itemText(0) == ui->comboBox_filter->currentText() && !update) {
        /* store current position in textedit */
        ui->textEdit->setFocus();
        QTextCursor storeCursorPos = ui->textEdit->textCursor();
        ui->textEdit->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        ui->textEdit->moveCursor(QTextCursor::Down, QTextCursor::KeepAnchor);
        ui->textEdit->textCursor().removeSelectedText();
        ui->textEdit->textCursor().deletePreviousChar();
        /* reset courser */
        ui->textEdit->setTextCursor(storeCursorPos);
    }

    filters->remove(ui->comboBox_filter->currentText());
    if (!update) {
        ui->comboBox_filter->removeItem(ui->comboBox_filter->currentIndex());
        ui->textEdit->textCursor().deletePreviousChar();
    }
}

void LogstorageConfigCreatorForm::on_pushButton_Add_clicked()
{
    if (validateFilter())
    {
        LogstorageFilter filter;

        if(filters->contains(ui->comboBox_filter->currentText())) /* remove filter */
        {
            /* store current position in textedit */
            ui->textEdit->setFocus();
            QTextCursor storeCursorPos = ui->textEdit->textCursor();

            remove_filter(true);

            filter.setApid(ui->lineEdit_apid->text());
            filter.setCtid(ui->lineEdit_ctid->text());
            filter.setLogLevel(ui->comboBox_level->currentText());
            filter.setFileName(ui->lineEdit_fname->text());
            filter.setFileSize(ui->lineEdit_fsize->text().toInt());
            filter.setNoFiles(ui->lineEdit_nofiles->text().toInt());

            filters->insert(ui->comboBox_filter->currentText(), filter);

            ui->textEdit->textCursor().insertText(filterToText(ui->comboBox_filter->currentText()));
            ui->textEdit->textCursor().insertText(QString("\n"));
            ui->textEdit->textCursor().insertText(filter.toText());

            /* reset courser */
            ui->textEdit->setTextCursor(storeCursorPos);
        }
        else /* add a new at the end */
        {
            ui->textEdit->moveCursor(QTextCursor::End);
            filter.setApid(ui->lineEdit_apid->text());
            filter.setCtid(ui->lineEdit_ctid->text());
            filter.setLogLevel(ui->comboBox_level->currentText());
            filter.setFileName(ui->lineEdit_fname->text());
            filter.setFileSize(ui->lineEdit_fsize->text().toInt());
            filter.setNoFiles(ui->lineEdit_nofiles->text().toInt());

            filters->insert(ui->comboBox_filter->currentText(), filter);

            ui->textEdit->textCursor().insertText(filterToText(ui->comboBox_filter->currentText()));
            ui->textEdit->textCursor().insertText(QString("\n"));
            ui->textEdit->textCursor().insertText(filter.toText());
            ui->textEdit->textCursor().insertText(QString("\n"));
        }

        // update "Add" to "Update" text of Add button
        set_button_text_Add_to_Update(0);
    }
}

void LogstorageConfigCreatorForm::on_pushButton_Delete_clicked()
{
    remove_filter(false);

    // make index 0 active
    ui->comboBox_filter->setCurrentIndex(0);
    load_filter(0);

}

void LogstorageConfigCreatorForm::on_pushButton_SaveFile_clicked()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as... (ONLY dlt_logstorage.conf is accepted by DLT Daemon)"),
                                                  QString("dlt_logstorage.conf"), tr("Logstorage Conf files (*.conf)"));
    QFile file(fn);

    QFileInfo fi(file);

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        // error handling here
        QMessageBox msgBox;
        msgBox.setText("Cannot open file.(" + file.errorString() + ")");
        msgBox.exec();
        return;
    }

    QTextStream out(&file);
    out << ui->textEdit->toPlainText();

    if(!file.flush()) {
        // error handling here
        QMessageBox msgBox;
        msgBox.setText("Flush Error.(" + file.errorString() + ")");
        msgBox.exec();
    }

    QMessageBox msgBox;
    msgBox.setText("File successfully saved: " + fi.absoluteFilePath());
    msgBox.exec();

    file.close();
}

void LogstorageConfigCreatorForm::on_pushButton_LoadFile_clicked()
{
    /* cleanup current filters */
    ui->comboBox_filter->clear();
    setFilterDefaults();
    filters->clear();
    ui->textEdit->clear();

    /* Load a filter configuration from a specified file */
    QString fn = QFileDialog::getOpenFileName(this, tr("Load a dlt_logstorage.conf file"));

    QFile file(fn);
    QFileInfo fi(file);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        // error handling here
        QMessageBox msgBox;
        msgBox.setText("Cannot open file.(" + file.errorString() + ")");
        msgBox.exec();
        return;
    }

    QSettings settings(fi.absoluteFilePath(), QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    foreach (QString g, groups) /* get filter configurations */
    {
        settings.beginGroup(g);
        ui->comboBox_filter->addItem(g);
        int index = ui->comboBox_filter->findText(g);
        if ( index != -1 ) // -1 for not found
            ui->comboBox_filter->setCurrentIndex(index);

        QString num = g;
        g.replace("FILTER","");
        currentFilter = g.toInt();

        QStringList keys = settings.childKeys();
        foreach (QString k, keys) {
            if (k.compare("LogAppName") == 0) {
                ui->lineEdit_apid->setText(settings.value(k).toString());
                //filter.setApid(settings.value(k).toString().toLatin1());
            }
            else if (k.compare("ContextName") == 0) {
                ui->lineEdit_ctid->setText(settings.value(k).toString());
                //filter.setCtid(settings.value(k).toString().toLatin1());
            }
            else if (k.compare("LogLevel") == 0) {
                int index = ui->comboBox_level->findText(settings.value(k).toString());
                if ( index != -1 ) // -1 for not found
                    ui->comboBox_level->setCurrentIndex(index);
                else
                    ui->comboBox_level->setCurrentIndex(3);
            }
            else if (k.compare("File") == 0) {
                ui->lineEdit_fname->setText(settings.value(k).toString());
            }
            else if (k.compare("FileSize") == 0) {
                ui->lineEdit_fsize->setText(settings.value(k).toString());
            }
            else if (k.compare("NOFiles") == 0) {
                ui->lineEdit_nofiles->setText(settings.value(k).toString());
            }
        }

        on_pushButton_Add_clicked();

        settings.endGroup();
    }

    file.close();
}

void LogstorageConfigCreatorForm::load_filter(int idx)
{

    LogstorageFilter filter = filters->value(ui->comboBox_filter->currentText());

    ui->lineEdit_apid->setText(filter.getApid());
    ui->lineEdit_ctid->setText(filter.getCtid());

    idx = ui->comboBox_level->findText(filter.getLogLevel());
    if ( idx != -1 ) // -1 for not found
        ui->comboBox_level->setCurrentIndex(idx);

    ui->lineEdit_fname->setText(filter.getFileName());
    ui->lineEdit_fsize->setText(QString::number(filter.getFileSize()));
    ui->lineEdit_nofiles->setText(QString::number(filter.getNoFiles()));
}

void LogstorageConfigCreatorForm::setFilterDefaults()
{
    ui->lineEdit_apid->setText(QString(""));
    ui->lineEdit_ctid->setText(QString(""));

    int index = ui->comboBox_level->findText(QString("DLT_LOG_INFO"));
    if ( index != -1 ) // -1 for not found
        ui->comboBox_level->setCurrentIndex(index);

    ui->lineEdit_fname->setText(QString(""));
    ui->lineEdit_fsize->setText(QString("50000"));
    ui->lineEdit_nofiles->setText(QString("5"));
}

bool LogstorageConfigCreatorForm::validateFilter()
{

    /* check for wildcards */
    if(ui->lineEdit_apid->text().contains(QString(".*")) && ui->lineEdit_ctid->text().contains(".*"))
    {
        QMessageBox msgBox;
        msgBox.setText("Wildcard for apid and ctid is prohibited.");
        msgBox.exec();

        ui->lineEdit_apid->setFocus();
        return false;
    }

    /* check if length bigger 2 and ".*" inside */
    if(ui->lineEdit_apid->text().length() > 2 && ui->lineEdit_apid->text().indexOf(QString(".")) != -1)
    {
        QMessageBox msgBox;
        msgBox.setText("Application Id is incorrect. Possibly . or .* within other apids.");
        msgBox.exec();

        ui->lineEdit_apid->setFocus();
        return false;
    }

    QStringList apids = ui->lineEdit_apid->text().split(QString(","));

    foreach(QString s, apids)
    {
        if(s.length() < 1 || s.length() > 4) {
            QMessageBox msgBox;
            msgBox.setText("One or more Application Ids have an invalid length.");
            msgBox.exec();
            ui->lineEdit_apid->setFocus();
            return false;
        }
    }

    /* check if length bigger 2 and ".*" inside */
    if(ui->lineEdit_ctid->text().length() > 2 && ui->lineEdit_ctid->text().indexOf(QString(".")) != -1)
    {
        QMessageBox msgBox;
        msgBox.setText("Context Id is incorrect. Possibly . or .* within other apids.");
        msgBox.exec();

        ui->lineEdit_ctid->setFocus();
        return false;
    }

    QStringList ctids = ui->lineEdit_ctid->text().split(QString(","));

    foreach(QString s, ctids)
    {
        if(s.length() < 1 || s.length() > 4) {
            QMessageBox msgBox;
            msgBox.setText("One or more Context Ids have an invalid length.");
            msgBox.exec();
            ui->lineEdit_ctid->setFocus();
            return false;
        }
    }

    /* check for filename */
    if(ui->lineEdit_fname->text().length() < 3)
    {
        QMessageBox msgBox;
        QString fn("Filename not long enough: ");
        fn.append(ui->lineEdit_fname->text());
        msgBox.setText(fn);
        msgBox.exec();

        ui->lineEdit_fname->setFocus();
        return false;
    }

    return true;
}
