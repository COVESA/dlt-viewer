#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>

namespace Ui {
    class FilterDialog;
}

class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QWidget *parent = 0);
    ~FilterDialog();

    void setEcuId(QString id);
    QString getEcuId();
    void setEnableEcuId(bool state);
    bool getEnableEcuId();

    void setApplicationId(QString id);
    QString getApplicationId();
    void setEnableApplicationId(bool state);
    bool getEnableApplicationId();

    void setContextId(QString id);
    QString getContextId();
    void setEnableContextId(bool state);
    bool getEnableContextId();

    void setHeaderText(QString id);
    QString getHeaderText();
    void setEnableHeaderText(bool state);
    bool getEnableHeaderText();

    void setPayloadText(QString id);
    QString getPayloadText();
    void setEnablePayloadText(bool state);
    bool getEnablePayloadText();

private:
    Ui::FilterDialog *ui;
};

#endif // FILTERDIALOG_H
