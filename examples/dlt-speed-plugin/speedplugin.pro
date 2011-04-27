TEMPLATE  = lib

CONFIG   += plugin

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

TARGET = $$qtLibraryTarget(speedplugin)

# Defines and Header Directories
DEFINES  += QT_VIEWER

INCLUDEPATH += ../../src \
            ../../qdlt

# Project files
HEADERS +=  plugininterface.h \
            ../../qdlt/qdlt.h \
            form.h \
            speedplugin.h

SOURCES += speedplugin.cpp \
    form.cpp

FORMS += \
    form.ui
