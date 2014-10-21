#ifndef DLTDBUSCATALOG_H
#define DLTDBUSCATALOG_H

#include <QList>
#include <QString>
#include <QStringList>

class DltDBusCatalogArgument
{
public:
    DltDBusCatalogArgument();
    ~DltDBusCatalogArgument();

    QString name;
    QString type;
    QString access;
    QString direction;
};

class DltDBusCatalogFunction
{
public:
    DltDBusCatalogFunction();
    ~DltDBusCatalogFunction();

    QList<DltDBusCatalogArgument*> arguments;

    QString name;
};

class DltDBusCatalogInterface
{
public:
    DltDBusCatalogInterface();
    ~DltDBusCatalogInterface();

    QList<DltDBusCatalogFunction*> functions;

    QString name;
};


class DltDBusCatalog
{
public:
    DltDBusCatalog();
    ~DltDBusCatalog();

    void clear();

    bool parse(QString filename);
    QStringList info();

    QList<DltDBusCatalogInterface*> interfaces;
    QString errorString;
};

#endif // DLTDBUSCATALOG_H
