#ifndef JUMPTODIALOG_H
#define JUMPTODIALOG_H

#include <QDialog>

namespace Ui {
class JumpToDialog;
}

class JumpToDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit JumpToDialog(QWidget *parent = 0);
    ~JumpToDialog();
    void setLimits(int min, int max);
    int getIndex();
    void showEvent(QShowEvent * event);
private:
    Ui::JumpToDialog *ui;
    int searchMin;
    int searchMax;
private slots:
    void validate();
};

#endif // JUMPTODIALOG_H
