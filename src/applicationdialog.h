#ifndef APPLICATIONDIALOG_H
#define APPLICATIONDIALOG_H

#include <QDialog>

namespace Ui {
    class ApplicationDialog;
}

class ApplicationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApplicationDialog(QString id,QString description,QWidget *parent = 0);
    ~ApplicationDialog();

    QString id();
    QString description();

private:
    Ui::ApplicationDialog *ui;
};

#endif // APPLICATIONDIALOG_H
