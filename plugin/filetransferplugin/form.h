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
    QTreeWidget* getTreeWidget();

private:
    Ui::Form *ui;

    int selectedFiles;

public slots:
    void selectAllClicked();
    void deselectAllClicked();
    void clearAllClicked();

    void saveClicked();

    void itemChanged(QTreeWidgetItem* item,int);
    void itemDoubleClicked ( QTreeWidgetItem * item, int column );
    void sectionInTableDoubleClicked(int logicalIndex);

private slots:
    void on_treeWidget_customContextMenuRequested(QPoint pos);
    void on_actionDelete_triggered();
    void on_actionSave_triggered();
};

#endif // FORM_H
