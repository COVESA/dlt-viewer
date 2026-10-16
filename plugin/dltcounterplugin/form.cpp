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
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>

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
    QStandardItemModel *missingDataModel = new QStandardItemModel();

    QProgressDialog progress("Finding missing data...", "Cancel", 0, plugin->consolidatedMap.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);


    missingDataModel->blockSignals(true);
    missingDataModel->setRowCount(0);

    int step = 0;

    QList<QStandardItem*> bulkItems;

    for (auto it = plugin->consolidatedMap.cbegin(); it != plugin->consolidatedMap.cend(); ++it) {
        const QString &ctid = it.key();
        const QSet<int> &counterSet = it.value();

        // Convert QSet to QList and sort
        QList<int> sortedCounters = counterSet.values();
        std::sort(sortedCounters.begin(), sortedCounters.end());

        int expected = 1;

        for (int counter : sortedCounters) {
            while (expected < counter) {
                QList<QStandardItem*> rowItems;

                QStandardItem *ctidItem = new QStandardItem(ctid);
                ctidItem->setTextAlignment(Qt::AlignCenter);

                QStandardItem *counterItem = new QStandardItem(QString::number(expected));
                counterItem->setTextAlignment(Qt::AlignCenter);

                rowItems.append(ctidItem);
                rowItems.append(counterItem);

                missingDataModel->appendRow(rowItems);
                expected++;
            }
            expected = counter + 1;
        }

               // Fill any remaining missing values up to 255
        for (; expected <= 255; ++expected) {
            bulkItems.append(new QStandardItem(ctid));
            bulkItems.append(new QStandardItem(QString::number(expected)));
        }

        progress.setValue(++step);

        if (progress.wasCanceled())
            break;
    }

    progress.setValue(plugin->consolidatedMap.size());

    // Batch append rows to the model
    for (int i = 0; i < bulkItems.size(); i += 2) {
        QList<QStandardItem*> rowItems;
        rowItems.append(bulkItems[i]);
        rowItems.append(bulkItems[i + 1]);
        missingDataModel->appendRow(rowItems);
    }

    missingDataModel->blockSignals(false);


    QTableView *sortedTableview = new QTableView();
    sortedTableview->setModel(missingDataModel);

    // Adjust the table view to be responsive

    sortedTableview->resize(400,800);
    sortedTableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sortedTableview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sortedTableview->setColumnWidth(0, 120);
    sortedTableview->setColumnWidth(1, 120);

    missingDataModel->setColumnCount(2);
    missingDataModel->setHorizontalHeaderLabels({"Ctid", "Missing Counter"});

    sortedTableview->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(sortedTableview, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos) {
        QModelIndex index = sortedTableview->indexAt(pos);
        if (!index.isValid()) return;

        int row = index.row();
        QString ctid = missingDataModel->item(row, 0)->text();
        int missingCounter = missingDataModel->item(row, 1)->text().toInt();

        QMenu contextMenu;
        QAction *prevAction = contextMenu.addAction("Previous");
        QAction *nextAction = contextMenu.addAction("Next");

        QAction *selectedAction = contextMenu.exec(sortedTableview->viewport()->mapToGlobal(pos));
        if (selectedAction == prevAction) {
            handlePrevious(ctid, missingCounter);
        } else if (selectedAction == nextAction) {
            handleNext(ctid, missingCounter);
        }
    });

    sortedTableview->show();
}

void Form::handlePrevious(const QString &ctid, int missingCounter)
{
    int prevCounter = findPreviousCounter(ctid, missingCounter);
    if (prevCounter != -1) {
        plugin->scrollToCounterInMainTable(ctid, prevCounter);
    } else {
        qDebug() << "No previous counter found.";
        QMessageBox::information(nullptr, "Not Found", "No matching value found in main table.");
    }
}

void Form::handleNext(const QString &ctid, int missingCounter)
{
    int nextCounter = findNextCounter(ctid, missingCounter);
    if (nextCounter != -1) {
        plugin->scrollToCounterInMainTable(ctid, nextCounter);
    } else {
        qDebug() << "No next counter found.";
        QMessageBox::information(nullptr, "Not Found", "No matching value found in main table.");
    }
}

int Form::findPreviousCounter(const QString &ctid, int counter)
{
    if (!plugin->consolidatedMap.contains(ctid)) return -1;

    const QSet<int> &counterSet = plugin->consolidatedMap[ctid];
    int bestMatch = -1;

    for (int val : counterSet) {
        if (val < counter && val > bestMatch) {
            bestMatch = val;
        }
    }

    return bestMatch;
}

int Form::findNextCounter(const QString &ctid, int counter)
{
    if (!plugin->consolidatedMap.contains(ctid)) return -1;
    const QSet<int> &counterSet = plugin->consolidatedMap[ctid];
    for (int i = counter + 1; i <= 9999999; ++i) { // Large limit
        if (counterSet.contains(i))
            return i;
    }
    return -1;
}

//Export Button : It exports the data to the local system.
//                The exported file will be generated in the current path of the .exe
//                The exported file is in .csv format

void Form::on_ExportpushButton_clicked()
{
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
    // QString filePath = QDir::currentPath() + "/" + "MissingCounter.csv";

    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save CSV File"),
        QDir::homePath() + "/MissingCounter.csv",
        tr("CSV Files (*.csv);;All Files (*)")
        );

    if (filePath.isEmpty()) {
        // User cancelled the dialog
        qDebug() << "Save cancelled by user.";
        return;
    }

    // Ensure the filename ends with .csv
    if (!filePath.endsWith(".csv", Qt::CaseInsensitive)) {
        filePath += ".csv";
    }

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
    qDebug() << "Missing Counters file successfully exported to:" << filePath;
}
