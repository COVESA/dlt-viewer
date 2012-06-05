/**
 * @file main.cpp
 * @brief Main file.
 * @author Micha? Policht
 */

#include "qextserialenumerator.h"
#include <QtCore/QList>
#include <QtCore/QDebug>
//! [0]
int main()
{
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    qDebug() << "List of ports:";
    for (int i = 0; i < ports.size(); i++) {
        qDebug() << "port name:" << ports.at(i).portName;
        qDebug() << "friendly name:" << ports.at(i).friendName;
        qDebug() << "physical name:" << ports.at(i).physName;
        qDebug() << "enumerator name:" << ports.at(i).enumName;
        qDebug() << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
        qDebug() << "product ID:" << QString::number(ports.at(i).productID, 16);
        qDebug() << "===================================";
    }
    return 0;
}
//! [0]
