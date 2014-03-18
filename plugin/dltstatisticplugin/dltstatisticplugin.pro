# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

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
