#ifndef SEARCHFORM_H
#define SEARCHFORM_H

#include <QWidget>

namespace Ui {
class SearchForm;
}

class QLineEdit;

class SearchForm : public QWidget
{
    Q_OBJECT

public:
    enum class State {
        INPUT,
        PROGRESS
    };

    explicit SearchForm(QWidget *parent = nullptr);
    ~SearchForm();

    QLineEdit* input() const;

    void setState(State state);
    void setProgress(int val);
    void resetProgress();

signals:
    void abortSearch();

private:
    Ui::SearchForm *ui;
};

#endif // SEARCHFORM_H
