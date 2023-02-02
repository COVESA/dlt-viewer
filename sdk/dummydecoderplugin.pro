TEMPLATE  = lib

CONFIG   += plugin

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

CONFIG(debug, debug|release) {
    DESTDIR = ../../../plugins
    QMAKE_LIBDIR += ../../lib
    LIBS += -lqdltd
}
else {
    DESTDIR = ../../../plugins
    QMAKE_LIBDIR += ../../lib
    LIBS += -lqdlt
}

TARGET = $$qtLibraryTarget(dummydecoderplugin)

# Defines and Header Directories
DEFINES  += QT_VIEWER

INCLUDEPATH += ../../include /usr/include/qdlt

# Project files
HEADERS += dummydecoderplugin.h \
            ../../include/qdlt/qdlt.h \
            ../../include/qdlt/plugininterface.h

SOURCES += dummydecoderplugin.cpp
