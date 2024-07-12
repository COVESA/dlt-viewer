QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Executable name
TARGET = dlt-commander

# Local includes
INCLUDEPATH = . ../qdlt

# Unix executable install path
target.path = $$PREFIX/usr/bin
INSTALLS += target

# Library definitions for debug and release builds
CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    QMAKE_LIBDIR += ../debug
    LIBS += -lqdltd
} else {
    DESTDIR = ../release
    QMAKE_LIBDIR += ../release
    QMAKE_RPATHDIR += ../build/release
    LIBS += -lqdlt
}

SOURCES += \
        main.cpp \
        optmanager.cpp

HEADERS += \
    export_rules.h \
    optmanager.h
