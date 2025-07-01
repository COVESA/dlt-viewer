# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# target name
TARGET = $$qtLibraryTarget(dltcounterplugin)

# plugin header files
HEADERS += \
    dltcounterplugin.h \
    form.h

# plugin source files
SOURCES += \
    dltcounterplugin.cpp \
    form.cpp

# plugin forms
FORMS += \
    form.ui
