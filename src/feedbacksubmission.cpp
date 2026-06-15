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
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSysInfo>
#include <QTextEdit>
#include <QUrl>
#include <QUrlQuery>
#include <QPushButton>
#include <QVBoxLayout>

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

    const QString dltViewerVersion = QString::fromLatin1(PACKAGE_VERSION);
    QLineEdit *versionEdit = new QLineEdit(&feedbackDialog);
    versionEdit->setText(dltViewerVersion);
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
        const QString title = titleEdit->text().trimmed();
        const QString description = descriptionEdit->toPlainText().trimmed();
        const QString emailId = emailEdit->text().trimmed();

        if (title.isEmpty() || description.isEmpty() || emailId.isEmpty()) {
            QMessageBox::warning(&feedbackDialog, "Submit Feedback",
                                "Please complete all required fields before submitting.");
            return;
        }

        const QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(emailId).hasMatch()) {
            QMessageBox::warning(&feedbackDialog, "Submit Feedback",
                                 "Please enter a valid email address.");
            return;
        }

        const QString category = categoryCombo->currentText();
        const QString operatingSystem = osEdit->text();
        const QString severity = severityCombo->currentText();

        QString body = QString("Feedback Category: %1\n\n").arg(category);
        body += QString("Title: %1\n\n").arg(title);
        body += QString("Description:\n%1\n\n").arg(description);
        body += QString("DLT Version: %1-%2-%3\n")
                .arg(dltViewerVersion)
                .arg(PACKAGE_VERSION_STATE)
                .arg(PACKAGE_REVISION);
        body += QString("Build Date: %1-%2\n")
            .arg(QString(__DATE__))
            .arg(QString(__TIME__));
        body += QString("Qt Version: %1\n").arg(QString(QT_VERSION_STR));
        body += QString("Operating System: %1\n").arg(operatingSystem);
        body += QString("Severity: %1\n").arg(severity);
        body += QString("Submitted By: %1\n").arg(emailId);

        const QString subject = "DLT Viewer Feedback: " + title;

        QUrl mailToUrl;
        mailToUrl.setScheme("mailto");
        mailToUrl.setPath(DLT_SUPPORT_MAIL_ADDRESS);

        QUrlQuery query;
        query.addQueryItem("Subject", subject);
        query.addQueryItem("body", body);
        mailToUrl.setQuery(query);

        const QDir sourceDir(QFileInfo(QString::fromLatin1(__FILE__)).absolutePath());
        QString readmePath = sourceDir.filePath("README.md");
        if (!QFileInfo::exists(readmePath)) {
            readmePath = sourceDir.absoluteFilePath("../README.md");
        }
        const QString readmeUrl = QUrl::fromLocalFile(readmePath).toString(QUrl::FullyEncoded);

        QMessageBox msgBox(&feedbackDialog);
        msgBox.setWindowTitle("Information");
        msgBox.setTextFormat(Qt::RichText); // this makes links clickable
        msgBox.setTextInteractionFlags(Qt::TextBrowserInteraction);
        msgBox.setText(QString("Support information can be found in <a href=\"%1\">ReadMe.md</a>")
                   .arg(readmeUrl.toHtmlEscaped()));
        if (QLabel *msgLabel = msgBox.findChild<QLabel *>("qt_msgbox_label")) {
            msgLabel->setOpenExternalLinks(true);
            msgLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        }
        QPushButton *generateButton = msgBox.addButton("Generate", QMessageBox::AcceptRole);
        msgBox.addButton(QMessageBox::Cancel);
        msgBox.exec();

        if (msgBox.clickedButton() == generateButton) {
            QDesktopServices::openUrl(mailToUrl);
            feedbackDialog.accept();
        }
    });

    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &feedbackDialog, &QDialog::reject);

    feedbackDialog.exec();
}
