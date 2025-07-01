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
 * \file form.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "form.h"
#include "ui_form.h"
#include "dltcounterplugin.h"

#include <qfiledialog.h>
#include <QStandardItemModel>
#include <QTableView>

using namespace DltCounter;

Form::Form(DltCounterPlugin *_plugin,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    plugin = _plugin;
}

Form::~Form()
{
    delete ui;
}

//CounterPushButton : Displays a window with counter value and its respective
//                    context ID. The counter values and the context IDs displayed
//                    are the missing values in the opened log file.

void Form::on_CounterpushButton_clicked()
{

    plugin->dataConsolidatedMap();
    QStandardItemModel *missingDataModel = new QStandardItemModel();

    // Iterate through the map and remove entries with an empty key
    auto it = plugin->consolidatedMap.begin();
    while (it != plugin->consolidatedMap.end()) {
        if (it.key().isEmpty()) { // Check if the key is empty
            it = plugin->consolidatedMap.erase(it); // Remove the entry and update the iterator
        } else {
            ++it; // Move to the next entry
        }
    }

    missingDataModel->setColumnCount(2);
    missingDataModel->setHorizontalHeaderLabels({"Ctid", "Missing Counter"});

    for (auto it = plugin->consolidatedMap.cbegin(); it != plugin->consolidatedMap.cend(); ++it) {
        QString ctid = it.key();
        QList<int> counters = it.value().values();

               // Sort the counter values
        std::sort(counters.begin(), counters.end());

               // Ensure that 1 is the starting point and 255 is the max value
        int expectedValue = 1;

        for (int counter : counters) {
            // If expectedValue is less than the current counter, those values are missing
            while (expectedValue < counter) {
                QList<QStandardItem*> rowItems;
                rowItems.append(new QStandardItem(ctid));
                rowItems.append(new QStandardItem(QString::number(expectedValue)));
                missingDataModel->appendRow(rowItems);

                expectedValue++;  // Move to the next expected value
            }
            expectedValue = counter + 1;  // Move to the next expected value after the current counter
        }
        while (expectedValue <= 255) {
            QList<QStandardItem*> rowItems;
            rowItems.append(new QStandardItem(ctid));
            rowItems.append(new QStandardItem(QString::number(expectedValue)));
            missingDataModel->appendRow(rowItems);
            expectedValue++;
        }
    }

    QTableView *sortedTableview = new QTableView();
    sortedTableview->setModel(missingDataModel);

           // Adjust the table view to be responsive
    sortedTableview->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents); // Adjust table to contents

    sortedTableview->setWindowTitle("Sorted Counter and Ctid Columns");
    sortedTableview->show();
}

//Export Button : It exports the data to the local system.
//                The exported file will be generated in the current path of the .exe
//                The exported file is in .csv format

void Form::on_ExportpushButton_clicked()
{
    plugin->dataConsolidatedMap();
    QStandardItemModel *missingDataModel = new QStandardItemModel();

    // Iterate through the map and remove entries with an empty key
    auto it = plugin->consolidatedMap.begin();
    while (it != plugin->consolidatedMap.end()) {
        if (it.key().isEmpty()) { // Check if the key is empty
            it = plugin->consolidatedMap.erase(it); // Remove the entry and update the iterator
        } else {
            ++it; // Move to the next entry
        }
    }

    missingDataModel->setColumnCount(2);
    missingDataModel->setHorizontalHeaderLabels({"Ctid", "Missing Counter"});

    for (auto it = plugin->consolidatedMap.cbegin(); it != plugin->consolidatedMap.cend(); ++it) {
        QString ctid = it.key();
        QList<int> counters = it.value().values();

               // Sort the counter values
        std::sort(counters.begin(), counters.end());

               // Ensure that 1 is the starting point and 255 is the max value
        int expectedValue = 1;

        for (int counter : counters) {
            while (expectedValue < counter) {
                // Add missing values to the model
                QList<QStandardItem*> rowItems;
                rowItems.append(new QStandardItem(ctid));
                rowItems.append(new QStandardItem(QString::number(expectedValue)));
                missingDataModel->appendRow(rowItems);

                expectedValue++;  // Move to the next expected value
            }
            expectedValue = counter + 1;  // Move to the next expected value after the current counter
        }

               // Check for any missing values till 255
        while (expectedValue <= 255) {
            QList<QStandardItem*> rowItems;
            rowItems.append(new QStandardItem(ctid));
            rowItems.append(new QStandardItem(QString::number(expectedValue)));
            missingDataModel->appendRow(rowItems);
            expectedValue++;
        }
    }


           // Get the current directory and create file path
    QString filePath = QDir::currentPath() + "/" + "MissingCounter.csv";

    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << filePath;
    }
    QTextStream stream(&file);
    stream << "Ctid,Missing Counter\n";

           // Write data to CSV
    for (int row = 0; row < missingDataModel->rowCount(); ++row) {
        QString ctid = missingDataModel->item(row, 0)->text();
        QString missingCounter = missingDataModel->item(row, 1)->text();
        stream << ctid << "," << missingCounter << "\n";
    }

    file.close();
    qDebug() << "CSV file successfully exported to:" << filePath;
}
