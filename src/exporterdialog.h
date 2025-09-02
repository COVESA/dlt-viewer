#ifndef EXPORTERDIALOG_H
#define EXPORTERDIALOG_H

#include <QDialog>

#include "qdltexporter.h"

namespace Ui {
class ExporterDialog;
}

class ExporterDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ExporterDialog(QWidget *parent = 0);
    ~ExporterDialog();

    void setFormat(QDltExporter::DltExportFormat exportFormat);
    QDltExporter::DltExportFormat getFormat();

    void setSelection(QDltExporter::DltExportSelection exportSelection);
    QDltExporter::DltExportSelection getSelection();

    void getRange(unsigned long *start, unsigned long *stop);
    void setRange(unsigned long start, unsigned long stop);
    
private:
    Ui::ExporterDialog *ui;
};

#endif // EXPORTERDIALOG_H
