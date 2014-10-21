# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# target name
TARGET = $$qtLibraryTarget(dltdbusplugin)

# plugin header files
HEADERS += \
    dltdbusplugin.h \
    form.h \
    dbus.h \
    dltdbuscatalog.h

# plugin source files
SOURCES += \
    dltdbusplugin.cpp \
    form.cpp \
    dbus.cpp \
    dltdbuscatalog.cpp

# plugin forms
FORMS += \
    form.ui
