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
#include <QtCore/QThread>
#include <QtCore/QReadWriteLock>
#include <QtCore/QMutexLocker>
#include <QtCore/QDebug>
#include <QtCore/QRegExp>
#include <QtCore/QMetaType>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#  include <QtCore/QWinEventNotifier>
#  define WinEventNotifier QWinEventNotifier
#elif !defined(QESP_NO_QT4_PRIVATE)
#  include <QtCore/private/qwineventnotifier_p.h>
#  define WinEventNotifier QWinEventNotifier
#else
#  include "qextwineventnotifier_p.h"
#  define WinEventNotifier QextWinEventNotifier
#endif
void QextSerialPortPrivate::platformSpecificInit()
{
    Win_Handle=INVALID_HANDLE_VALUE;
    ZeroMemory(&overlap, sizeof(OVERLAPPED));
    overlap.hEvent = CreateEvent(NULL, true, false, NULL);
    winEventNotifier = 0;
    bytesToWriteLock = new QReadWriteLock;
    _bytesToWrite = 0;
}

void QextSerialPortPrivate::platformSpecificDestruct() {
    CloseHandle(overlap.hEvent);
    delete bytesToWriteLock;
}


/*!
    \internal
    COM ports greater than 9 need \\.\ prepended

    This is only need when open the port.
*/
static QString fullPortNameWin(const QString & name)
{
    QRegExp rx(QLatin1String("^COM(\\d+)"));
    QString fullName(name);
    if(fullName.contains(rx))
        fullName.prepend(QLatin1String("\\\\.\\"));
    return fullName;
}

bool QextSerialPortPrivate::open_sys(QIODevice::OpenMode mode)
{
    Q_Q(QextSerialPort);
    DWORD confSize = sizeof(COMMCONFIG);
    Win_CommConfig.dwSize = confSize;
    DWORD dwFlagsAndAttributes = 0;
    if (_queryMode == QextSerialPort::EventDriven)
        dwFlagsAndAttributes += FILE_FLAG_OVERLAPPED;

    /*open the port*/
    Win_Handle=CreateFileW((wchar_t*)fullPortNameWin(port).utf16(), GENERIC_READ|GENERIC_WRITE,
                           0, NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
    if (Win_Handle!=INVALID_HANDLE_VALUE) {
        q->setOpenMode(mode);
        /*configure port settings*/
        GetCommConfig(Win_Handle, &Win_CommConfig, &confSize);
        GetCommState(Win_Handle, &(Win_CommConfig.dcb));

        /*set up parameters*/
        Win_CommConfig.dcb.fBinary=TRUE;
        Win_CommConfig.dcb.fInX=FALSE;
        Win_CommConfig.dcb.fOutX=FALSE;
        Win_CommConfig.dcb.fAbortOnError=FALSE;
        Win_CommConfig.dcb.fNull=FALSE;
        /*flush all settings*/
        settingsDirtyFlags = DFE_ALL;
        updatePortSettings();

        //init event driven approach
        if (_queryMode == QextSerialPort::EventDriven) {
            if (!SetCommMask( Win_Handle, EV_TXEMPTY | EV_RXCHAR | EV_DSR)) {
                QESP_WARNING()<<"failed to set Comm Mask. Error code:"<<GetLastError();
                return false;
            }
            winEventNotifier = new WinEventNotifier(overlap.hEvent);
            qRegisterMetaType<HANDLE>("HANDLE");
            q->connect(winEventNotifier, SIGNAL(activated(HANDLE)), q, SLOT(_q_onWinEvent(HANDLE)), Qt::DirectConnection);
            WaitCommEvent(Win_Handle, &eventMask, &overlap);
        }
        return true;
    }
    return false;
}

bool QextSerialPortPrivate::close_sys()
{
    flush_sys();
    CancelIo(Win_Handle);
    if (CloseHandle(Win_Handle))
        Win_Handle = INVALID_HANDLE_VALUE;
    if (winEventNotifier){
        winEventNotifier->setEnabled(false);
        winEventNotifier->deleteLater();
        winEventNotifier = 0;
    }
    _bytesToWrite = 0;

    foreach(OVERLAPPED* o, pendingWrites) {
        CloseHandle(o->hEvent);
        delete o;
    }
    pendingWrites.clear();
    return true;
}

bool QextSerialPortPrivate::flush_sys()
{
    FlushFileBuffers(Win_Handle);
    return true;
}

qint64 QextSerialPortPrivate::bytesAvailable_sys() const
{
    DWORD Errors;
    COMSTAT Status;
    if (ClearCommError(Win_Handle, &Errors, &Status)) {
        return Status.cbInQue;
    }
    return (qint64)-1;
}

/*
    Translates a system-specific error code to a QextSerialPort error code.  Used internally.
*/
void QextSerialPortPrivate::translateError(ulong error)
{
    if (error&CE_BREAK) {
        lastErr=E_BREAK_CONDITION;
    }
    else if (error&CE_FRAME) {
        lastErr=E_FRAMING_ERROR;
    }
    else if (error&CE_IOE) {
        lastErr=E_IO_ERROR;
    }
    else if (error&CE_MODE) {
        lastErr=E_INVALID_FD;
    }
    else if (error&CE_OVERRUN) {
        lastErr=E_BUFFER_OVERRUN;
    }
    else if (error&CE_RXPARITY) {
        lastErr=E_RECEIVE_PARITY_ERROR;
    }
    else if (error&CE_RXOVER) {
        lastErr=E_RECEIVE_OVERFLOW;
    }
    else if (error&CE_TXFULL) {
        lastErr=E_TRANSMIT_OVERFLOW;
    }
}

/*
    Reads a block of data from the serial port.  This function will read at most maxlen bytes from
    the serial port and place them in the buffer pointed to by data.  Return value is the number of
    bytes actually read, or -1 on error.
    
    \warning before calling this function ensure that serial port associated with this class
    is currently open (use isOpen() function to check if port is open).
*/
qint64 QextSerialPortPrivate::readData_sys(char *data, qint64 maxSize)
{
    DWORD bytesRead = 0;
    bool failed = false;
    if (_queryMode == QextSerialPort::EventDriven) {
        OVERLAPPED overlapRead;
        ZeroMemory(&overlapRead, sizeof(OVERLAPPED));
        if (!ReadFile(Win_Handle, (void*)data, (DWORD)maxSize, & bytesRead, & overlapRead)) {
            if (GetLastError() == ERROR_IO_PENDING)
                GetOverlappedResult(Win_Handle, & overlapRead, & bytesRead, true);
            else
                failed = true;
        }
    } else if (!ReadFile(Win_Handle, (void*)data, (DWORD)maxSize, & bytesRead, NULL)) {
        failed = true;
    }
    if (!failed)
        return (qint64)bytesRead;

    lastErr = E_READ_FAILED;
    return -1;
}

/*
    Writes a block of data to the serial port.  This function will write len bytes
    from the buffer pointed to by data to the serial port.  Return value is the number
    of bytes actually written, or -1 on error.
    
    \warning before calling this function ensure that serial port associated with this class
    is currently open (use isOpen() function to check if port is open).
*/
qint64 QextSerialPortPrivate::writeData_sys(const char *data, qint64 maxSize)
{
    DWORD bytesWritten = 0;
    bool failed = false;
    if (_queryMode == QextSerialPort::EventDriven) {
        OVERLAPPED* newOverlapWrite = new OVERLAPPED;
        ZeroMemory(newOverlapWrite, sizeof(OVERLAPPED));
        newOverlapWrite->hEvent = CreateEvent(NULL, true, false, NULL);
        if (WriteFile(Win_Handle, (void*)data, (DWORD)maxSize, & bytesWritten, newOverlapWrite)) {
            CloseHandle(newOverlapWrite->hEvent);
            delete newOverlapWrite;
        }
        else if (GetLastError() == ERROR_IO_PENDING) {
            // writing asynchronously...not an error
            QWriteLocker writelocker(bytesToWriteLock);
            _bytesToWrite += maxSize;
            pendingWrites.append(newOverlapWrite);
        }
        else {
            QESP_WARNING()<<"QextSerialPort write error:"<<GetLastError();
            failed = true;
            if(!CancelIo(newOverlapWrite->hEvent))
                QESP_WARNING("QextSerialPort: couldn't cancel IO");
            if(!CloseHandle(newOverlapWrite->hEvent))
                QESP_WARNING("QextSerialPort: couldn't close OVERLAPPED handle");
            delete newOverlapWrite;
        }
    } else if (!WriteFile(Win_Handle, (void*)data, (DWORD)maxSize, & bytesWritten, NULL)) {
        failed = true;
    }

    if (!failed)
        return (qint64)bytesWritten;

    lastErr = E_WRITE_FAILED;
    return -1;
}

void QextSerialPortPrivate::setDtr_sys(bool set) {
    EscapeCommFunction(Win_Handle, set ? SETDTR : CLRDTR);
}

void QextSerialPortPrivate::setRts_sys(bool set) {
    EscapeCommFunction(Win_Handle, set ? SETRTS : CLRRTS);
}

ulong QextSerialPortPrivate::lineStatus_sys(void) {
    unsigned long Status=0, Temp=0;
    GetCommModemStatus(Win_Handle, &Temp);
    if (Temp & MS_CTS_ON) Status|=LS_CTS;
    if (Temp & MS_DSR_ON) Status|=LS_DSR;
    if (Temp & MS_RING_ON) Status|=LS_RI;
    if (Temp & MS_RLSD_ON) Status|=LS_DCD;
    return Status;
}

/*
  Triggered when there's activity on our HANDLE.
*/
void QextSerialPortPrivate::_q_onWinEvent(HANDLE h)
{
    Q_Q(QextSerialPort);
    if(h == overlap.hEvent) {
        if (eventMask & EV_RXCHAR) {
            if (q->sender() != q && bytesAvailable_sys() > 0)
                _q_canRead();
        }
        if (eventMask & EV_TXEMPTY) {
            /*
              A write completed.  Run through the list of OVERLAPPED writes, and if
              they completed successfully, take them off the list and delete them.
              Otherwise, leave them on there so they can finish.
            */
            qint64 totalBytesWritten = 0;
            QList<OVERLAPPED*> overlapsToDelete;
            foreach(OVERLAPPED* o, pendingWrites) {
                DWORD numBytes = 0;
                if (GetOverlappedResult(Win_Handle, o, & numBytes, false)) {
                    overlapsToDelete.append(o);
                    totalBytesWritten += numBytes;
                } else if( GetLastError() != ERROR_IO_INCOMPLETE ) {
                    overlapsToDelete.append(o);
                    QESP_WARNING()<<"CommEvent overlapped write error:" << GetLastError();
                }
            }

            if (q->sender() != q && totalBytesWritten > 0) {
                QWriteLocker writelocker(bytesToWriteLock);
                Q_EMIT q->bytesWritten(totalBytesWritten);
                _bytesToWrite = 0;
            }

            foreach(OVERLAPPED* o, overlapsToDelete) {
                OVERLAPPED *toDelete = pendingWrites.takeAt(pendingWrites.indexOf(o));
                CloseHandle(toDelete->hEvent);
                delete toDelete;
            }
        }
        if (eventMask & EV_DSR) {
            if (lineStatus_sys() & LS_DSR)
                Q_EMIT q->dsrChanged(true);
            else
                Q_EMIT q->dsrChanged(false);
        }
    }
    WaitCommEvent(Win_Handle, &eventMask, &overlap);
}

void QextSerialPortPrivate::updatePortSettings()
{
    if (!q_ptr->isOpen() || !settingsDirtyFlags)
        return;

    //fill struct : COMMCONFIG
    if (settingsDirtyFlags & DFE_BaudRate) {
        Win_CommConfig.dcb.BaudRate = Settings.BaudRate;
    }
    if (settingsDirtyFlags & DFE_Parity) {
        Win_CommConfig.dcb.Parity = (BYTE)Settings.Parity;
        Win_CommConfig.dcb.fParity = (Settings.Parity == PAR_NONE) ? FALSE : TRUE;
    }
    if (settingsDirtyFlags & DFE_DataBits) {
        Win_CommConfig.dcb.ByteSize = (BYTE)Settings.DataBits;
    }
    if (settingsDirtyFlags & DFE_StopBits) {
        switch (Settings.StopBits) {
        case STOP_1:
            Win_CommConfig.dcb.StopBits = ONESTOPBIT;
            break;
        case STOP_1_5:
            Win_CommConfig.dcb.StopBits = ONE5STOPBITS;
            break;
        case STOP_2:
            Win_CommConfig.dcb.StopBits = TWOSTOPBITS;
            break;
        }
    }
    if (settingsDirtyFlags & DFE_Flow) {
        switch(Settings.FlowControl) {
        /*no flow control*/
        case FLOW_OFF:
            Win_CommConfig.dcb.fOutxCtsFlow=FALSE;
            Win_CommConfig.dcb.fRtsControl=RTS_CONTROL_DISABLE;
            Win_CommConfig.dcb.fInX=FALSE;
            Win_CommConfig.dcb.fOutX=FALSE;
            break;
        /*software (XON/XOFF) flow control*/
        case FLOW_XONXOFF:
            Win_CommConfig.dcb.fOutxCtsFlow=FALSE;
            Win_CommConfig.dcb.fRtsControl=RTS_CONTROL_DISABLE;
            Win_CommConfig.dcb.fInX=TRUE;
            Win_CommConfig.dcb.fOutX=TRUE;
            break;
        /*hardware flow control*/
        case FLOW_HARDWARE:
            Win_CommConfig.dcb.fOutxCtsFlow=TRUE;
            Win_CommConfig.dcb.fRtsControl=RTS_CONTROL_HANDSHAKE;
            Win_CommConfig.dcb.fInX=FALSE;
            Win_CommConfig.dcb.fOutX=FALSE;
            break;
        }
    }

    //fill struct : COMMTIMEOUTS
    if (settingsDirtyFlags & DFE_TimeOut) {
        if (_queryMode != QextSerialPort::EventDriven) {
            int millisec = Settings.Timeout_Millisec;
            if (millisec == -1) {
                Win_CommTimeouts.ReadIntervalTimeout = MAXDWORD;
                Win_CommTimeouts.ReadTotalTimeoutConstant = 0;
            } else {
                Win_CommTimeouts.ReadIntervalTimeout = millisec;
                Win_CommTimeouts.ReadTotalTimeoutConstant = millisec;
            }
            Win_CommTimeouts.ReadTotalTimeoutMultiplier = 0;
            Win_CommTimeouts.WriteTotalTimeoutMultiplier = millisec;
            Win_CommTimeouts.WriteTotalTimeoutConstant = 0;
        }
        else {
            Win_CommTimeouts.ReadIntervalTimeout = MAXDWORD;
            Win_CommTimeouts.ReadTotalTimeoutMultiplier = 0;
            Win_CommTimeouts.ReadTotalTimeoutConstant = 0;
            Win_CommTimeouts.WriteTotalTimeoutMultiplier = 0;
            Win_CommTimeouts.WriteTotalTimeoutConstant = 0;
        }
    }


    if (settingsDirtyFlags & DFE_Settings_Mask)
        SetCommConfig(Win_Handle, &Win_CommConfig, sizeof(COMMCONFIG));
    if ((settingsDirtyFlags & DFE_TimeOut))
        SetCommTimeouts(Win_Handle, &Win_CommTimeouts);
    settingsDirtyFlags = 0;
}
