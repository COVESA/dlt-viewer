#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QTreeWidget>
#include "file.h"
namespace Ui {
    class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
    void makeConnections();
    QTreeWidget* getTreeWidget();

public slots:
    void expandClicked();
    void collapseClicked();

    void selectAllClicked();
    void deselectAllClicked();

    void saveClicked();

    void itemChanged(QTreeWidgetItem* item,int);
    void itemDoubleClicked ( QTreeWidgetItem * item, int column );

private:
    Ui::Form *ui;
};

#endif // FORM_H
