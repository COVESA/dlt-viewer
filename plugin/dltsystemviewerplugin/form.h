#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "plugininterface.h"

namespace Ui {
    class Form;
}

class ProcessItem  : public QTreeWidgetItem
{
public:

    ProcessItem(QTreeWidgetItem *parent = 0);
    ~ProcessItem();

    unsigned int lastTimestamp;
};

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    void addProcesses(int pid, QString data,QDltMsg &msg);

    void setUser(QString text);
    void setNice(QString text);
    void setSystem(QString text);

    void deleteAllProccesses();

private slots:
    void on_pushButtonClear_clicked();

private:
    Ui::Form *ui;

};

#endif // FORM_H
