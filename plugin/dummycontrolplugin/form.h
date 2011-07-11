#ifndef FORM_H
#define FORM_H

#include <QWidget>

class DummyControlPlugin;

namespace Ui {
    class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(DummyControlPlugin *_plugin, QWidget *parent = 0);
    ~Form();

    void setConnections(QStringList list);

private:
    Ui::Form *ui;
    DummyControlPlugin *plugin;

private slots:
    void on_pushButton_clicked();

};

#endif // FORM_H
