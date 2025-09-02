#include "searchform.h"
#include "ui_searchform.h"

#include <QLineEdit>
#include <QCompleter>
#include <QSettings>

SearchForm::SearchForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchForm)
{
    ui->setupUi(this);

    ui->comboBox->setLineEdit(new QLineEdit);
    ui->comboBox->setInsertPolicy(QComboBox::InsertAtTop);

    m_completer = new QCompleter(&m_historyModel, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
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
        if(list.size() > MaxComboBoxHistorySize){
        list.removeFirst();
        m_historyModel.setStringList(std::move(list));
        }
    }
}

void SearchForm::saveComboBoxSearchHistory() {
    //To save the search history of QCombobox
    QStringList saveList = m_historyModel.stringList();
    QSettings settings("MyData", "SearchList");
    settings.beginWriteArray("HistoryList");
    int count = qMin(saveList.size(), 20);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue("entryList", saveList.at(i));
    }
    settings.endArray();
}

void SearchForm::loadComboBoxSearchHistory()
{
    //To retrive the search history once DLT Viewer restarts
    QStringList loadList = m_historyModel.stringList();
    QSettings settings("MyData", "SearchList");
    loadList.clear();
    int size = settings.beginReadArray("HistoryList");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        loadList.append(settings.value("entryList").toString());
    }
    settings.endArray();
    m_historyModel.setStringList(loadList);
    QStringList revLoadList = loadList;
    std::reverse(revLoadList.begin(), revLoadList.end());
    ui->comboBox->addItems(revLoadList);
    input()->clear();
}

