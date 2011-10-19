#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include "qdlt.h"
#include <QTableView>
#include <QTreeWidget>
namespace Ui {
    class SearchDialog;
}


class SearchDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::SearchDialog *ui;
    int startLine;
    bool nextClicked;
    bool match;
    bool onceClicked;


public:
    explicit SearchDialog(QWidget *parent = 0);
    ~SearchDialog();

    void setText(QString text);
    void selectText();
    void setHeader(bool header);
    void setPayload(bool payload);
    void setCaseSensitive(bool caseSensitive);
    void setRegExp(bool regExp);
    void setMatch(bool matched);
    void setStartLine(int start);
    void setNextClicked(bool next);
    void setSearchColour(QLineEdit *lineEdit,int result);
    void setEnabledPrevious(bool enabled);
    void setSearchPosition(bool beginning);
    void setOnceClicked(bool clicked);
    void appendLineEdit(QLineEdit *lineEdit);

    QString getText();
    bool getHeader();
    bool getPayload();
    bool getCaseSensitive();
    bool getRegExp();
    bool getSearchFromBeginning();
    bool getNextClicked();
    bool getMatch();
    bool getClicked();
    bool getOnceClicked();
    int getStartLine();
    int find();
    QDltFile *file;
    QTableView *table;
    QTreeWidget *plugin;
    QList<QLineEdit*> *lineEdits;

private slots:
    void on_lineEditText_textEdited(QString newText);
    void on_pushButtonPrevious_clicked();
    void on_pushButtonNext_clicked();
public slots:
    void textEditedFromToolbar(QString newText);
    void findNextClicked();
    void findPreviousClicked();
};

#endif // SEARCHDIALOG_H
