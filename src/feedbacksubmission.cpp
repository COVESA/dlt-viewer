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
 * \file feedbacksubmission.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "feedbacksubmission.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSysInfo>
#include <QTextEdit>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <qpushbutton.h>

#include "version.h"

void FeedbackSubmission::showDialog(QWidget *parent)
{
    QDialog feedbackDialog(parent);
    feedbackDialog.setWindowTitle("Submit Feedback");
    feedbackDialog.setMinimumWidth(550);

    QVBoxLayout *mainLayout = new QVBoxLayout(&feedbackDialog);

    QLabel *instructionLabel = new QLabel("Fields marked with * are required.", &feedbackDialog);
    instructionLabel->setWordWrap(true);
    mainLayout->addWidget(instructionLabel);

    QFormLayout *formLayout = new QFormLayout();

    QComboBox *categoryCombo = new QComboBox(&feedbackDialog);
    categoryCombo->addItems({"Bug", "Feature", "Plugin"});
    formLayout->addRow("Category *", categoryCombo);

    QLineEdit *titleEdit = new QLineEdit(&feedbackDialog);
    titleEdit->setPlaceholderText("Short summary of the issue or request");
    formLayout->addRow("Title *", titleEdit);

    QTextEdit *descriptionEdit = new QTextEdit(&feedbackDialog);
    descriptionEdit->setPlaceholderText("Describe the feedback in detail");
    descriptionEdit->setMinimumHeight(120);
    formLayout->addRow("Description *", descriptionEdit);

    QLineEdit *versionEdit = new QLineEdit(&feedbackDialog);
    versionEdit->setText(PACKAGE_VERSION);
    versionEdit->setReadOnly(true);
    formLayout->addRow("DLT Viewer Version *", versionEdit);

    QLineEdit *osEdit = new QLineEdit(&feedbackDialog);
    osEdit->setText(QSysInfo::prettyProductName());
    osEdit->setReadOnly(true);
    formLayout->addRow("Operating System *", osEdit);

    QComboBox *severityCombo = new QComboBox(&feedbackDialog);
    severityCombo->addItems({"Low", "Medium", "High", "Critical"});
    formLayout->addRow("Severity *", severityCombo);

    QLineEdit *emailEdit = new QLineEdit(&feedbackDialog);
    emailEdit->setPlaceholderText("name@example.com");
    formLayout->addRow("Email ID *", emailEdit);

    mainLayout->addLayout(formLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &feedbackDialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Generate Email");
    mainLayout->addWidget(buttonBox);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &feedbackDialog, [&]() {
        QString title = titleEdit->text().trimmed();
        QString description = descriptionEdit->toPlainText().trimmed();
        QString emailId = emailEdit->text().trimmed();

        if (title.isEmpty() || description.isEmpty() || emailId.isEmpty()) {
            QMessageBox::warning(&feedbackDialog, "Submit Feedback",
                                "Please complete all required fields before submitting.");
            return;
        }

        QString category = categoryCombo->currentText();
        QString version = versionEdit->text();
        QString operatingSystem = osEdit->text();
        QString severity = severityCombo->currentText();

        QString body = "Feedback Category: " + category + "\n\n";
        body += "Title: " + title + "\n\n";
        body += "Description:\n" + description + "\n\n";
        body += "DLT Version: " + version + "-" + PACKAGE_VERSION_STATE + "-" + PACKAGE_REVISION + "\n";
        body += "Build Date: " + QString(__DATE__) + "-" + QString(__TIME__) + "\n";
        body += "Qt Version: " + QString(QT_VERSION_STR) + "\n";
        body += "Operating System: " + operatingSystem + "\n";
        body += "Severity: " + severity + "\n";
        body += "Submitted By: " + emailId + "\n";

        QString subject = "DLT Viewer Feedback: " + title;

        QUrl mailToUrl;
        mailToUrl.setScheme("mailto");
        mailToUrl.setPath(DLT_SUPPORT_MAIL_ADDRESS);

        QUrlQuery query;
        query.addQueryItem("Subject", subject);
        query.addQueryItem("body", body);
        mailToUrl.setQuery(query);

        const QString encodedMailto = mailToUrl.toString(QUrl::FullyEncoded);

        QMessageBox msgBox(parent);
        msgBox.setWindowTitle("Mail-Support DLT");
        msgBox.setTextFormat(Qt::RichText); //this is what makes the links clickable
        QString text = "<a href=\"";
        text.append(encodedMailto.toHtmlEscaped());
        text.append("\"><center>Mailto ").append(QString(DLT_SUPPORT_NAME).toHtmlEscaped()).append(" DLT-Viewer-Support:<br>");
        text.append(QString(DLT_SUPPORT_MAIL_ADDRESS).toHtmlEscaped()).append("</center></a>");
        msgBox.setText(text);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        feedbackDialog.accept();
    });

    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &feedbackDialog, &QDialog::reject);

    feedbackDialog.exec();
}
