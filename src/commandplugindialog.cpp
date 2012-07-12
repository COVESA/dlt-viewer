#include "commandplugindialog.h"
#include "ui_commandplugindialog.h"

CommandPluginDialog::CommandPluginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandPluginDialog)
{
    ui->setupUi(this);
    connect(ui->btnAddParam, SIGNAL(clicked()), this, SLOT(addParam()));
}

CommandPluginDialog::~CommandPluginDialog()
{
    delete ui;
}

void CommandPluginDialog::setPlugin(QDltPluginCommandInterface *aPlugin)
{
    this->mPlugin = aPlugin;
    if(mPlugin)
    {
        populateCommands();
    }
}

QDltPluginCommandInterface *CommandPluginDialog::plugin()
{
    return this->mPlugin;
}

void CommandPluginDialog::populateCommands()
{
    QList<QString> cmds = this->mPlugin->commandList();
    ui->cbCommands->clear();
    ui->cbCommands->addItems(cmds);
    mCommand = ui->cbCommands->currentText();
}

void CommandPluginDialog::commandChanged(QString cmd)
{
    this->mCommand = cmd;
}

QString CommandPluginDialog::command()
{
    return mCommand;
}

QList<QString> CommandPluginDialog::params()
{
    QList<QString> ret;
    for(int i=0;i < ui->listParams->count();i++)
    {
        ret.append(ui->listParams->item(i)->text());
    }
    return ret;
}

void CommandPluginDialog::addParam()
{
    QListWidgetItem *item = new QListWidgetItem("new parameter");
    item->setFlags(item->flags () | Qt::ItemIsEditable);
    ui->listParams->insertItem(ui->listParams->count(), item);
}
