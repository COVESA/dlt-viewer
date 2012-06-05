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

#include "qextserialport.h"
#include "qextserialport_p.h"
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <QtCore/QMutexLocker>
#include <QtCore/QDebug>
#include <QtCore/QSocketNotifier>

void QextSerialPortPrivate::platformSpecificInit()
{
    fd = 0;
    readNotifier = 0;
}

/*!
    Standard destructor.
*/
void QextSerialPortPrivate::platformSpecificDestruct()
{
}

bool QextSerialPortPrivate::open_sys(QIODevice::OpenMode mode)
{
    Q_Q(QextSerialPort);
    //note: linux 2.6.21 seems to ignore O_NDELAY flag
    if ((fd = ::open(port.toAscii() ,O_RDWR | O_NOCTTY | O_NDELAY)) != -1) {

        /*In the Private class, We can not call QIODevice::open()*/
        q->setOpenMode(mode);             // Flag the port as opened
        ::tcgetattr(fd, &old_termios);    // Save the old termios
        Posix_CommConfig = old_termios;   // Make a working copy
        ::cfmakeraw(&Posix_CommConfig);   // Enable raw access

        /*set up other port settings*/
        Posix_CommConfig.c_cflag |= CREAD|CLOCAL;
        Posix_CommConfig.c_lflag &= (~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ISIG));
        Posix_CommConfig.c_iflag &= (~(INPCK|IGNPAR|PARMRK|ISTRIP|ICRNL|IXANY));
        Posix_CommConfig.c_oflag &= (~OPOST);
        Posix_CommConfig.c_cc[VMIN] = 0;
#ifdef _POSIX_VDISABLE  // Is a disable character available on this system?
        // Some systems allow for per-device disable-characters, so get the
        //  proper value for the configured device
        const long vdisable = ::fpathconf(fd, _PC_VDISABLE);
        Posix_CommConfig.c_cc[VINTR] = vdisable;
        Posix_CommConfig.c_cc[VQUIT] = vdisable;
        Posix_CommConfig.c_cc[VSTART] = vdisable;
        Posix_CommConfig.c_cc[VSTOP] = vdisable;
        Posix_CommConfig.c_cc[VSUSP] = vdisable;
#endif //_POSIX_VDISABLE
        settingsDirtyFlags = DFE_ALL;
        updatePortSettings();

        if (_queryMode == QextSerialPort::EventDriven) {
            readNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, q);
            q->connect(readNotifier, SIGNAL(activated(int)), q, SLOT(_q_canRead()));
        }
        return true;
    } else {
        translateError(errno);
        return false;
    }
}

bool QextSerialPortPrivate::close_sys()
{
    // Force a flush and then restore the original termios
    flush_sys();
    // Using both TCSAFLUSH and TCSANOW here discards any pending input
    ::tcsetattr(fd, TCSAFLUSH | TCSANOW, &old_termios);   // Restore termios
    ::close(fd);
    if(readNotifier) {
        delete readNotifier;
        readNotifier = 0;
    }
    return true;
}

bool QextSerialPortPrivate::flush_sys()
{
    ::tcdrain(fd);
    return true;
}

qint64 QextSerialPortPrivate::bytesAvailable_sys() const
{
    int bytesQueued;
    if (::ioctl(fd, FIONREAD, &bytesQueued) == -1) {
        return (qint64)-1;
    }
    return bytesQueued;
}

/*!
    Translates a system-specific error code to a QextSerialPort error code.  Used internally.
*/
void QextSerialPortPrivate::translateError(ulong error)
{
    switch (error) {
    case EBADF:
    case ENOTTY:
        lastErr = E_INVALID_FD;
        break;
    case EINTR:
        lastErr = E_CAUGHT_NON_BLOCKED_SIGNAL;
        break;
    case ENOMEM:
        lastErr = E_NO_MEMORY;
        break;
    case EACCES:
        lastErr = E_PERMISSION_DENIED;
        break;
    case EAGAIN:
        lastErr = E_AGAIN;
        break;
    }
}

void QextSerialPortPrivate::setDtr_sys(bool set)
{
    int status;
    ::ioctl(fd, TIOCMGET, &status);
    if (set)
        status |= TIOCM_DTR;
    else
        status &= ~TIOCM_DTR;
    ::ioctl(fd, TIOCMSET, &status);
}

void QextSerialPortPrivate::setRts_sys(bool set)
{
    int status;
    ::ioctl(fd, TIOCMGET, &status);
    if (set)
        status |= TIOCM_RTS;
    else
        status &= ~TIOCM_RTS;
    ::ioctl(fd, TIOCMSET, &status);
}

unsigned long QextSerialPortPrivate::lineStatus_sys()
{
    unsigned long Status=0, Temp=0;
    ::ioctl(fd, TIOCMGET, &Temp);
    if (Temp & TIOCM_CTS) Status |= LS_CTS;
    if (Temp & TIOCM_DSR) Status |= LS_DSR;
    if (Temp & TIOCM_RI ) Status |= LS_RI;
    if (Temp & TIOCM_CD ) Status |= LS_DCD;
    if (Temp & TIOCM_DTR) Status |= LS_DTR;
    if (Temp & TIOCM_RTS) Status |= LS_RTS;
    if (Temp & TIOCM_ST ) Status |= LS_ST;
    if (Temp & TIOCM_SR ) Status |= LS_SR;
    return Status;
}

/*!
    Reads a block of data from the serial port.  This function will read at most maxSize bytes from
    the serial port and place them in the buffer pointed to by data.  Return value is the number of
    bytes actually read, or -1 on error.
    
    \warning before calling this function ensure that serial port associated with this class
    is currently open (use isOpen() function to check if port is open).
*/
qint64 QextSerialPortPrivate::readData_sys(char * data, qint64 maxSize)
{
    int retVal = ::read(fd, data, maxSize);
    if (retVal == -1)
        lastErr = E_READ_FAILED;

    return retVal;
}

/*!
    Writes a block of data to the serial port.  This function will write maxSize bytes
    from the buffer pointed to by data to the serial port.  Return value is the number
    of bytes actually written, or -1 on error.
    
    \warning before calling this function ensure that serial port associated with this class
    is currently open (use isOpen() function to check if port is open).
*/
qint64 QextSerialPortPrivate::writeData_sys(const char * data, qint64 maxSize)
{
    int retVal = ::write(fd, data, maxSize);
    if (retVal == -1)
        lastErr = E_WRITE_FAILED;

    return (qint64)retVal;
}

static void setBaudRate2Termios(termios *config, int baudRate)
{
#ifdef CBAUD
    config->c_cflag &= (~CBAUD);
    config->c_cflag |= baudRate;
#else
    ::cfsetispeed(config, baudRate);
    ::cfsetospeed(config, baudRate);
#endif
}

/*
    All the platform settings was performed in this function.
*/
void QextSerialPortPrivate::updatePortSettings()
{
    if (!q_func()->isOpen() || !settingsDirtyFlags)
        return;

    if (settingsDirtyFlags & DFE_BaudRate) {
        switch (Settings.BaudRate) {
        case BAUD50:
            setBaudRate2Termios(&Posix_CommConfig, B50);
            break;
        case BAUD75:
            setBaudRate2Termios(&Posix_CommConfig, B75);
            break;
        case BAUD110:
            setBaudRate2Termios(&Posix_CommConfig, B110);
            break;
        case BAUD134:
            setBaudRate2Termios(&Posix_CommConfig, B134);
            break;
        case BAUD150:
            setBaudRate2Termios(&Posix_CommConfig, B150);
            break;
        case BAUD200:
            setBaudRate2Termios(&Posix_CommConfig, B200);
            break;
        case BAUD300:
            setBaudRate2Termios(&Posix_CommConfig, B300);
            break;
        case BAUD600:
            setBaudRate2Termios(&Posix_CommConfig, B600);
            break;
        case BAUD1200:
            setBaudRate2Termios(&Posix_CommConfig, B1200);
            break;
        case BAUD1800:
            setBaudRate2Termios(&Posix_CommConfig, B1800);
            break;
        case BAUD2400:
            setBaudRate2Termios(&Posix_CommConfig, B2400);
            break;
        case BAUD4800:
            setBaudRate2Termios(&Posix_CommConfig, B4800);
            break;
        case BAUD9600:
            setBaudRate2Termios(&Posix_CommConfig, B9600);
            break;
        case BAUD19200:
            setBaudRate2Termios(&Posix_CommConfig, B19200);
            break;
        case BAUD38400:
            setBaudRate2Termios(&Posix_CommConfig, B38400);
            break;
        case BAUD57600:
            setBaudRate2Termios(&Posix_CommConfig, B57600);
            break;
#ifdef B76800
        case BAUD76800:
            setBaudRate2Termios(&Posix_CommConfig, B76800);
            break;
#endif
        case BAUD115200:
            setBaudRate2Termios(&Posix_CommConfig, B115200);
            break;
#if defined(B230400) && defined(B4000000)
        case BAUD230400:
            setBaudRate2Termios(&Posix_CommConfig, B230400);
            break;
        case BAUD460800:
            setBaudRate2Termios(&Posix_CommConfig, B460800);
            break;
        case BAUD500000:
            setBaudRate2Termios(&Posix_CommConfig, B500000);
            break;
        case BAUD576000:
            setBaudRate2Termios(&Posix_CommConfig, B576000);
            break;
        case BAUD921600:
            setBaudRate2Termios(&Posix_CommConfig, B921600);
            break;
        case BAUD1000000:
            setBaudRate2Termios(&Posix_CommConfig, B1000000);
            break;
        case BAUD1152000:
            setBaudRate2Termios(&Posix_CommConfig, B1152000);
            break;
        case BAUD1500000:
            setBaudRate2Termios(&Posix_CommConfig, B1500000);
            break;
        case BAUD2000000:
            setBaudRate2Termios(&Posix_CommConfig, B2000000);
            break;
        case BAUD2500000:
            setBaudRate2Termios(&Posix_CommConfig, B2500000);
            break;
        case BAUD3000000:
            setBaudRate2Termios(&Posix_CommConfig, B3000000);
            break;
        case BAUD3500000:
            setBaudRate2Termios(&Posix_CommConfig, B3500000);
            break;
        case BAUD4000000:
            setBaudRate2Termios(&Posix_CommConfig, B4000000);
            break;
#endif
        }
    }
    if (settingsDirtyFlags & DFE_Parity) {
        switch (Settings.Parity) {
        case PAR_SPACE:
            /*space parity not directly supported - add an extra data bit to simulate it*/
            settingsDirtyFlags |= DFE_DataBits;
            break;
        case PAR_NONE:
            Posix_CommConfig.c_cflag &= (~PARENB);
            break;
        case PAR_EVEN:
            Posix_CommConfig.c_cflag &= (~PARODD);
            Posix_CommConfig.c_cflag |= PARENB;
            break;
        case PAR_ODD:
            Posix_CommConfig.c_cflag |= (PARENB|PARODD);
            break;
        }
    }
    /*must after Parity settings*/
    if (settingsDirtyFlags & DFE_DataBits) {
        if (Settings.Parity != PAR_SPACE) {
            Posix_CommConfig.c_cflag &= (~CSIZE);
            switch(Settings.DataBits) {
            case DATA_5:
                Posix_CommConfig.c_cflag |= CS5;
                break;
            case DATA_6:
                Posix_CommConfig.c_cflag |= CS6;
                break;
            case DATA_7:
                Posix_CommConfig.c_cflag |= CS7;
                break;
            case DATA_8:
                Posix_CommConfig.c_cflag |= CS8;
                break;
            }
        } else {
            /*space parity not directly supported - add an extra data bit to simulate it*/
            Posix_CommConfig.c_cflag &= ~(PARENB|CSIZE);
            switch(Settings.DataBits) {
            case DATA_5:
                Posix_CommConfig.c_cflag |= CS6;
                break;
            case DATA_6:
                Posix_CommConfig.c_cflag |= CS7;
                break;
            case DATA_7:
                Posix_CommConfig.c_cflag |= CS8;
                break;
            case DATA_8:
                /*this will never happen, put here to Suppress an warning*/
                break;
            }
        }
    }
    if (settingsDirtyFlags & DFE_StopBits) {
        switch (Settings.StopBits) {
        case STOP_1:
            Posix_CommConfig.c_cflag &= (~CSTOPB);
            break;
        case STOP_2:
            Posix_CommConfig.c_cflag |= CSTOPB;
            break;
        }
    }
    if (settingsDirtyFlags & DFE_Flow) {
        switch(Settings.FlowControl) {
        case FLOW_OFF:
            Posix_CommConfig.c_cflag &= (~CRTSCTS);
            Posix_CommConfig.c_iflag &= (~(IXON|IXOFF|IXANY));
            break;
        case FLOW_XONXOFF:
            /*software (XON/XOFF) flow control*/
            Posix_CommConfig.c_cflag &= (~CRTSCTS);
            Posix_CommConfig.c_iflag |= (IXON|IXOFF|IXANY);
            break;
        case FLOW_HARDWARE:
            Posix_CommConfig.c_cflag |= CRTSCTS;
            Posix_CommConfig.c_iflag &= (~(IXON|IXOFF|IXANY));
            break;
        }
    }

    /*if any thing in Posix_CommConfig changed, flush*/
    if (settingsDirtyFlags & DFE_Settings_Mask)
        ::tcsetattr(fd, TCSAFLUSH, &Posix_CommConfig);

    if (settingsDirtyFlags & DFE_TimeOut) {
        int millisec = Settings.Timeout_Millisec;
        if (millisec == -1) {
            ::fcntl(fd, F_SETFL, O_NDELAY);
        }
        else {
            //O_SYNC should enable blocking ::write()
            //however this seems not working on Linux 2.6.21 (works on OpenBSD 4.2)
            ::fcntl(fd, F_SETFL, O_SYNC);
        }
        ::tcgetattr(fd, & Posix_CommConfig);
        Posix_CommConfig.c_cc[VTIME] = millisec/100;
        ::tcsetattr(fd, TCSAFLUSH, & Posix_CommConfig);
    }

    settingsDirtyFlags = 0;
}
