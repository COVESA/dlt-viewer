#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QDialog>

namespace Ui {
    class PluginDialog;
}

class PluginDialog : public QDialog
{
    Q_OBJECT

public:

    explicit PluginDialog(QWidget *parent = 0);
    ~PluginDialog();

    void setName(QString name);
    void setMode(int mode);
    void setType(int type);
    void setFilename(QString filename);

    int getMode();
    int getType();
    QString getFilename();

private:
    Ui::PluginDialog *ui;

private slots:
    void on_toolButton_clicked();
};

#endif // PLUGINDIALOG_H
