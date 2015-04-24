#include <QMessageBox>

#include "jumptodialog.h"
#include "ui_jumptodialog.h"

JumpToDialog::JumpToDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JumpToDialog)
{
  ui->setupUi(this);

  Qt::WindowFlags flags = windowFlags();
  Qt::WindowFlags disableFlag = Qt::WindowContextHelpButtonHint;
  flags = flags & (~disableFlag);
  setWindowFlags(flags);

}

JumpToDialog::~JumpToDialog()
{
    delete ui;
}

void JumpToDialog::setLimits(int min, int max)
{
    searchMin = min;
    searchMax = max;
}

int JumpToDialog::getIndex()
{
    QString it = ui->lineEditIndex->text();
    bool ok;
    int ret = it.toInt(&ok);
    if(ok)
    {
        return ret;
    }
    else
    {
        return -1;
    }
}

void JumpToDialog::validate()
{
    if(getIndex() < 0)
    {
        QString err = "Invalid search index. Could not parse to valid number.";
        QMessageBox::warning(this, "Warning", err);
        return;
    }
    if(getIndex() < searchMin || getIndex() > searchMax)
    {
        QString err = "Invalid search index. The index must be between %1 and %2.";
        err = err.arg(searchMin).arg(searchMax);
        QMessageBox::warning(this, "Warning", err);
        return;
    }

    emit accept();
}

void JumpToDialog::showEvent(QShowEvent *)
{
    ui->lineEditIndex->setFocus();
}
