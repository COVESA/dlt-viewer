
PROJECT                 = qextserialport
TEMPLATE                = lib

target.path = /usr/local/lib
INSTALLS += target

#CONFIG                 -= debug_and_release
#CONFIG                 += debug
#CONFIG                 += release
CONFIG                 += warn_on qt thread
QT                     -= gui

OBJECTS_DIR             = build/obj
MOC_DIR                 = build/moc
DEPENDDIR               = .
INCLUDEDIR              = .
HEADERS                 = qextserialbase.h \
                          qextserialport.h
SOURCES                 = qextserialbase.cpp \
                          qextserialport.cpp

unix:HEADERS           += posix_qextserialport.h
unix:SOURCES           += posix_qextserialport.cpp
unix:DEFINES           += _TTY_POSIX_

win32:HEADERS          += win_qextserialport.h
win32:SOURCES          += win_qextserialport.cpp
win32:DEFINES          += _TTY_WIN_

unix:VERSION            = 1.1.0

CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    TARGET  = qextserialportd
}
else {
    DESTDIR = ../release
    TARGET  = qextserialport
}
