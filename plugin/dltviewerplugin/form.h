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

    void setTextBrowserHeader(QString text);
    void setTextBrowserPayload(QString text);
    void setTextBrowserAscii(QString text);
    void setTextBrowserBinary(QString text);
    void setTextBrowserMixed(QString text);

/*
    void setMessages(int message);
    void setSelectedMessage(int message);
    void setVerboseMessages(int message);
    void setNonVerboseMessages(int message);
*/

private:
    Ui::Form *ui;
};

#endif // FORM_H
