TEMPLATE  = lib

CONFIG   += plugin

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

TARGET = $$qtLibraryTarget(nonverboseplugin)

# Defines and Header Directories
DEFINES  += QT_VIEWER

INCLUDEPATH += ../../src \
            ../../qdlt

# Project files
HEADERS += nonverboseplugin.h \
    ../../src/plugininterface.h

SOURCES += nonverboseplugin.cpp

OTHER_FILES += \
    dlt-example-non-verbose-1.xml
