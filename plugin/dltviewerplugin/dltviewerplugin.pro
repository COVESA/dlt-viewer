TEMPLATE  = lib

CONFIG   += plugin

# QT += network

target.path = /usr/share/dlt-viewer/plugins
INSTALLS += target

CONFIG(debug, debug|release) {
    DESTDIR = ../../debug/plugins
    QMAKE_LIBDIR += ../../debug
    LIBS += -lqdltd
}
else {
    DESTDIR = ../../release/plugins
    QMAKE_LIBDIR += ../../release
    LIBS += -lqdlt
}

TARGET = $$qtLibraryTarget(dltviewerplugin)

# Defines and Header Directories
DEFINES  += QT_VIEWER

INCLUDEPATH += ../../src \
            ../../qdlt

# Project files
HEADERS += dltviewerplugin.h \
    form.h

SOURCES += dltviewerplugin.cpp \
    form.cpp

FORMS += \
    form.ui
