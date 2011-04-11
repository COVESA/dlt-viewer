#ifndef CONTEXTDIALOG_H
#define CONTEXTDIALOG_H

#include <QDialog>

namespace Ui {
    class ContextDialog;
}

class ContextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContextDialog(QString id,QString description,int loglevel, int tracestatus, QWidget *parent = 0);
    ~ContextDialog();

    QString id();
    QString description();
    int loglevel();
    int tracestatus();
    int update();

private:
    Ui::ContextDialog *ui;
};

#endif // CONTEXTDIALOG_H
