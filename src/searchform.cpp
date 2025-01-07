#include "searchform.h"
#include "ui_searchform.h"

#include <QLineEdit>
#include <QCompleter>

SearchForm::SearchForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchForm)
{
    ui->setupUi(this);

    ui->comboBox->setLineEdit(new QLineEdit);
    ui->comboBox->setInsertPolicy(QComboBox::InsertAtTop);

    m_completer = new QCompleter(&m_historyModel, this);
    input()->setCompleter(m_completer);

    connect (ui->abortButton, &QPushButton::clicked, this, &SearchForm::abortSearch);
}

SearchForm::~SearchForm()
{
    delete ui;
}

QLineEdit *SearchForm::input() const
{
    return ui->comboBox->lineEdit();
}

void SearchForm::setState(State state)
{
    switch(state) {
    case State::INPUT:
        ui->stackedWidget->setCurrentIndex(0);
        break;
    case State::PROGRESS:
        ui->stackedWidget->setCurrentIndex(1);
        break;
    }
}

void SearchForm::setProgress(int val)
{
    ui->progressBar->setValue(val);
}

void SearchForm::resetProgress()
{
    setProgress(0);
}

void SearchForm::updateHistory() {
    if (auto list = m_historyModel.stringList();
        !input()->text().isEmpty() && !list.contains(input()->text())) {
        list.append(input()->text());
        m_historyModel.setStringList(std::move(list));
    }
}
