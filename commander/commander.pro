QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Executable name
TARGET = dlt-commander

# Local includes
INCLUDEPATH = . ../qdlt

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

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    export_rules.h \
    optmanager.h
