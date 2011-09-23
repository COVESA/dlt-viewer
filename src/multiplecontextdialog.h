#ifndef MULTIPLECONTEXTDIALOG_H
#define MULTIPLECONTEXTDIALOG_H

#include <QDialog>

namespace Ui {
    class MultipleContextDialog;
}

class MultipleContextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultipleContextDialog(int loglevel, int tracestatus, QWidget *parent = 0);
    ~MultipleContextDialog();

    int loglevel();
    int tracestatus();
    int update();
private:
    Ui::MultipleContextDialog *ui;
};

#endif // MULTIPLECONTEXTDIALOG_H
