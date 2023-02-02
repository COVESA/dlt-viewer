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

TARGET = $$qtLibraryTarget(dummyviewerplugin)

# Defines and Header Directories
DEFINES  += QT_VIEWER

INCLUDEPATH += ../../include /usr/include/qdlt

# Project files
HEADERS +=  dummyviewerplugin.h \
            form.h \
            ../../include/qdlt/qdlt.h \
            ../../include/qdlt/plugininterface.h

SOURCES +=  dummyviewerplugin.cpp \
            form.cpp

FORMS +=    form.ui
