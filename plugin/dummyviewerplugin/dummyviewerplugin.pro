# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(dummyviewerplugin)

# plugin header files
HEADERS += \
    dummyviewerplugin.h \
    form.h

# plugin source files
SOURCES += \
    dummyviewerplugin.cpp \
    form.cpp

# plugin forms
FORMS += \
    form.ui
