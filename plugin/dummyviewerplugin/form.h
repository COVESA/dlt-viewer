#ifndef FORM_H
#define FORM_H

#include <QWidget>

namespace Ui {
    class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    void setMessages(int message);
    void setSelectedMessage(int message);
    void setVerboseMessages(int message);
    void setNonVerboseMessages(int message);

private:
    Ui::Form *ui;

};

#endif // FORM_H
