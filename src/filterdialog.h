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

    void setType(int value);
    int getType();

    void setName(QString name);
    QString getName();

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

    void setFilterColour(int value);
    int getFilterColour();

    void setLogLevelMax(int value);
    int getLogLevelMax();
    void setEnableLogLevelMax(bool state);
    bool getEnableLogLevelMax();

    void setLogLevelMin(int value);
    int getLogLevelMin();
    void setEnableLogLevelMin(bool state);
    bool getEnableLogLevelMin();

    void setEnableCtrlMsgs(bool state);
    bool getEnableCtrlMsgs();

private:
    Ui::FilterDialog *ui;
};

#endif // FILTERDIALOG_H
