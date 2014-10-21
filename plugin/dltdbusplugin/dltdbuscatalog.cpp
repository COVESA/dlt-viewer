#include "dltdbuscatalog.h"

#include <QFile>
#include <QXmlStreamReader>

DltDBusCatalogArgument::DltDBusCatalogArgument()
{

}
DltDBusCatalogArgument::~DltDBusCatalogArgument()
{

}

DltDBusCatalogFunction::DltDBusCatalogFunction()
{

}
DltDBusCatalogFunction::~DltDBusCatalogFunction()
{
    for (int i = 0; i < arguments.size(); ++i)
        delete arguments.at(i);
    arguments.clear();

}

DltDBusCatalogInterface::DltDBusCatalogInterface()
{

}

DltDBusCatalogInterface::~DltDBusCatalogInterface()
{
    for (int i = 0; i < functions.size(); ++i)
        delete functions.at(i);
    functions.clear();
}

DltDBusCatalog::DltDBusCatalog()
{
}

DltDBusCatalog::~DltDBusCatalog()
{
    clear();
}

void DltDBusCatalog::clear()
{
    for (int i = 0; i < interfaces.size(); ++i)
    {
        delete interfaces.at(i);
    }
    interfaces.clear();
}

bool DltDBusCatalog::parse(QString filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
            errorString = "Could not open File: ";
            errorString.append(filename).append(" for configuration.");

            return false;
    }

    QString warning_text;

    DltDBusCatalogInterface *interface = 0;
    DltDBusCatalogFunction  *function = 0;
    DltDBusCatalogArgument  *argument = 0;

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
          xml.readNext();

          if(xml.isStartElement())
          {
              if(xml.name() == QString("interface"))
              {
                  if(!interface)
                  {
                    interface = new DltDBusCatalogInterface();
                    interface->name = xml.attributes().value(QString("name")).toString();
                  }
              }
              if(xml.name() == QString("property")||xml.name() == QString("signal")||xml.name() == QString("method"))
              {
                  if(interface)
                  {
                    if(!function)
                    {
                        function = new DltDBusCatalogFunction();
                    }
                    function->name = xml.attributes().value(QString("name")).toString();
                    if(xml.name() == QString("property"))
                    {
                        argument = new DltDBusCatalogArgument();
                        argument->name = xml.attributes().value(QString("name")).toString();
                        argument->access = xml.attributes().value(QString("access")).toString();
                        argument->type = xml.attributes().value(QString("type")).toString();
                        function->arguments.append(argument);
                        argument = 0;
                    }
                  }
              }
              if(xml.name() == QString("arg"))
              {
                  if(function)
                  {
                    if(!argument)
                    {
                        argument = new DltDBusCatalogArgument();
                    }
                    argument->name = xml.attributes().value(QString("name")).toString();
                    argument->direction = xml.attributes().value(QString("direction")).toString();
                    argument->type = xml.attributes().value(QString("type")).toString();
                  }
              }
          }
          if(xml.isEndElement())
          {
              if(xml.name() == QString("interface"))
              {
                  if(interface)
                  {
                      interfaces.append(interface);
                      interface = 0;
                  }
              }
              if(xml.name() == QString("property")||xml.name() == QString("signal")||xml.name() == QString("method"))
              {
                  if(function)
                  {
                      interface->functions.append(function);
                      function = 0;
                  }
              }
              if(xml.name() == QString("arg"))
              {
                  if(argument)
                  {
                      if(argument->direction == "out")
                          function->arguments.append(argument);
                      else
                          function->arguments.append(argument);
                      argument = 0;
                  }
              }
          }
    }

    if(interface) delete interface;
    if(function) delete function;
    if(argument) delete argument;

    if (xml.hasError()) {

        errorString.append("\nXML Parser error: ").append(xml.errorString()).append("\n");

        file.close();
        return false;
    }

    file.close();
    return true;
}

QStringList DltDBusCatalog::info()
{
    QStringList list;

    for (int i = 0; i < interfaces.size(); ++i)
    {
        DltDBusCatalogInterface *ptr = interfaces.at(i);
        list.append(ptr->name);
        for (int j = 0; j < ptr->functions.size(); ++j)
        {
            DltDBusCatalogFunction *ptr2 =  ptr->functions.at(j);
            QString funcString;
            funcString = QString("   ") + ptr2->name + QString("(");
            bool first = true;
            for (int k = 0; k < ptr2->arguments.size(); ++k)
            {
                DltDBusCatalogArgument *ptr3 = ptr2->arguments.at(k);
                if(!first)
                    funcString += QString(",");
                else
                    first = false;
                funcString += ptr3->name;
            }
            funcString +=  QString(")");
            list.append(funcString);
        }

    }

    return list;
}
