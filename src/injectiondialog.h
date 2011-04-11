#ifndef INJECTIONDIALOG_H
#define INJECTIONDIALOG_H

#include <QDialog>

namespace Ui {
    class InjectionDialog;
}

class InjectionDialog : public QDialog {
    Q_OBJECT
public:
    InjectionDialog(QString appid,QString conid,QWidget *parent = 0);
    ~InjectionDialog();

    void setApplicationId(QString text);
    void setContextId(QString text);
    void setServiceId(QString text);
    void setData(QString text);

    QString getApplicationId();
    QString getContextId();
    QString getServiceId();
    QString getData();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::InjectionDialog *ui;
};

#endif // INJECTIONDIALOG_H
