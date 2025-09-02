#include "exporterdialog.h"
#include "ui_exporterdialog.h"
#include <QDebug>

ExporterDialog::ExporterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExporterDialog)
{
    ui->setupUi(this);
}

ExporterDialog::~ExporterDialog()
{
    delete ui;
}

void ExporterDialog::setFormat(QDltExporter::DltExportFormat exportFormat)
{
    if(exportFormat == QDltExporter::FormatDlt)
        ui->radioButtonDlt->setChecked(true);
    else if(exportFormat == QDltExporter::FormatAscii)
        ui->radioButtonAscii->setChecked(true);
    else if (exportFormat == QDltExporter::FormatUTF8)
        ui->radioButtonUTF8->setChecked(true);
    else if(exportFormat == QDltExporter::FormatCsv)
        ui->radioButtonCsv->setChecked(true);
    else if(exportFormat == QDltExporter::FormatDltDecoded)
        ui->radioButtonDltDecoded->setChecked(true);
}

QDltExporter::DltExportFormat ExporterDialog::getFormat()
{
    if(ui->radioButtonDlt->isChecked())
        return QDltExporter::FormatDlt;
    if(ui->radioButtonAscii->isChecked())
        return QDltExporter::FormatAscii;
    if(ui->radioButtonUTF8->isChecked())
        return QDltExporter::FormatUTF8;
    if(ui->radioButtonCsv->isChecked())
        return QDltExporter::FormatCsv;
    if(ui->radioButtonDltDecoded->isChecked())
        return QDltExporter::FormatDltDecoded;
    return QDltExporter::FormatDlt;
}

void ExporterDialog::setSelection(QDltExporter::DltExportSelection exportSelection)
{
    if(exportSelection == QDltExporter::SelectionAll)
        ui->radioButtonAll->setChecked(true);
    else if(exportSelection == QDltExporter::SelectionFiltered)
        ui->radioButtonFiltered->setChecked(true);
    else if(exportSelection == QDltExporter::SelectionSelected)
        ui->radioButtonSelection->setChecked(true);
}

QDltExporter::DltExportSelection ExporterDialog::getSelection()
{
    if(ui->radioButtonAll->isChecked())
        return QDltExporter::SelectionAll;
    if(ui->radioButtonFiltered->isChecked())
        return QDltExporter::SelectionFiltered;
    if(ui->radioButtonSelection->isChecked())
        return QDltExporter::SelectionSelected;
    return QDltExporter::SelectionAll;
}

void ExporterDialog::getRange(unsigned long *start, unsigned long *stop)
{
    *start=ui->startindex->text().toLongLong();
    *stop=ui->stopindex->text().toLongLong();
}

void ExporterDialog::setRange(unsigned long start, unsigned long stop)
{
    ui->startindex->setText(QString::number(start));
    ui->stopindex->setText(QString::number(stop));
}
