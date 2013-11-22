# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(dltstatisticplugin)

# plugin header files
HEADERS += \
    dltstatisticplugin.h \
    form.h \
    qdlttimelinewidget.h

# plugin source files
SOURCES += \
    dltstatisticplugin.cpp \
    form.cpp \
    qdlttimelinewidget.cpp

# plugin forms
FORMS += \
    form.ui
