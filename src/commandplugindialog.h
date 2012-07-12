#ifndef COMMANDPLUGINDIALOG_H
#define COMMANDPLUGINDIALOG_H

#include <QDialog>
#include <QModelIndex>
#include "plugininterface.h"

namespace Ui {
class CommandPluginDialog;
}

class CommandPluginDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CommandPluginDialog(QWidget *parent = 0);
    ~CommandPluginDialog();
    void setPlugin(QDltPluginCommandInterface *aPlugin);
    QDltPluginCommandInterface *plugin();
    QString command();
    QList<QString> params();
    
private:
    Ui::CommandPluginDialog *ui;
    QDltPluginCommandInterface *mPlugin;
    void populateCommands();
    QString mCommand;
    QList<QString> mParams;

private slots:
    void commandChanged(QString cmd);
    void addParam();
};

#endif // COMMANDPLUGINDIALOG_H
