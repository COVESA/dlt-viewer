/****************************************************************************
** Copyright (c) 2000-2003 Wayne Roth
** Copyright (c) 2004-2007 Stefan Sander
** Copyright (c) 2007 Michal Policht
** Copyright (c) 2008 Brandon Fosdick
** Copyright (c) 2009-2010 Liam Staskawicz
** Copyright (c) 2011 Debao Zhang
** All right reserved.
** Web: http://code.google.com/p/qextserialport/
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/

#include "qextserialenumerator.h"
#include "qextserialenumerator_p.h"
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QDir>

void QextSerialEnumeratorPrivate::platformSpecificInit()
{
}

void QextSerialEnumeratorPrivate::platformSpecificDestruct()
{
}

QList<QextPortInfo> QextSerialEnumeratorPrivate::getPorts_sys()
{
    QList<QextPortInfo> infoList;
#ifdef Q_OS_LINUX
    QStringList portNamePrefixes, portNameList;
    portNamePrefixes << QLatin1String("ttyS*"); // list normal serial ports first

    QDir dir(QLatin1String("/dev"));
    portNameList = dir.entryList(portNamePrefixes, (QDir::System | QDir::Files), QDir::Name);

    // remove the values which are not serial ports for e.g.  /dev/ttysa
    for (int i = 0; i < portNameList.size(); i++) {
        bool ok;
        QString current = portNameList.at(i);
        // remove the ttyS part, and check, if the other part is a number
        current.remove(0,4).toInt(&ok, 10);
        if (!ok) {
            portNameList.removeAt(i);
            i--;
        }
    }

    // get the non standard serial ports names
    // (USB-serial, bluetooth-serial, 18F PICs, and so on)
    // if you know an other name prefix for serial ports please let us know
    portNamePrefixes.clear();
    portNamePrefixes << QLatin1String("ttyACM*") << QLatin1String("ttyUSB*") << QLatin1String("rfcomm*");
    portNameList += dir.entryList(portNamePrefixes, (QDir::System | QDir::Files), QDir::Name);

    foreach (QString str , portNameList) {
        QextPortInfo inf;
        inf.physName = QLatin1String("/dev/")+str;
        inf.portName = str;

        if (str.contains(QLatin1String("ttyS"))) {
            inf.friendName = QLatin1String("Serial port ")+str.remove(0, 4);
        }
        else if (str.contains(QLatin1String("ttyUSB"))) {
            inf.friendName = QLatin1String("USB-serial adapter ")+str.remove(0, 6);
        }
        else if (str.contains(QLatin1String("rfcomm"))) {
            inf.friendName = QLatin1String("Bluetooth-serial adapter ")+str.remove(0, 6);
        }
        inf.enumName = QLatin1String("/dev"); // is there a more helpful name for this?
        infoList.append(inf);
    }
#endif
    return infoList;
}

bool QextSerialEnumeratorPrivate::setUpNotifications_sys(bool setup)
{
    Q_UNUSED(setup)
    return false;
}
