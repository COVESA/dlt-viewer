#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

namespace Ui {
    class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::SearchDialog *ui;

public:
    explicit SearchDialog(QWidget *parent = 0);
    ~SearchDialog();

    void setText(QString text);
    void setHeader(bool header);
    void setPayload(bool payload);
    void setCaseSensitive(bool caseSensitive);
    void setRegExp(bool regExp);

    QString getText();
    bool getHeader();
    bool getPayload();
    bool getCaseSensitive();
    bool getRegExp();

};

#endif // SEARCHDIALOG_H
