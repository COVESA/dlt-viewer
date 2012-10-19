# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(dummycontrolplugin)

# plugin header files
HEADERS += \
    dummycontrolplugin.h \
    form.h

# plugin source files
SOURCES += \
    dummycontrolplugin.cpp \
    form.cpp

# plugin forms
FORMS += \
    form.ui
