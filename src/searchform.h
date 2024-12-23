#ifndef SEARCHFORM_H
#define SEARCHFORM_H

#include <QWidget>
#include <QStringListModel>

namespace Ui {
class SearchForm;
}

class QLineEdit;
class QCompleter;

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
    void updateHistory();

signals:
    void abortSearch();

private:
    Ui::SearchForm *ui;

    QCompleter *m_completer{nullptr};
    QStringListModel m_historyModel;
};

#endif // SEARCHFORM_H
