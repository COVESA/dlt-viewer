# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# target name
TARGET = $$qtLibraryTarget(nonverboseplugin)

# plugin header files
HEADERS += \
    nonverboseplugin.h

# plugin source files
SOURCES += \
    nonverboseplugin.cpp

# plugin forms
FORMS +=

# other files
OTHER_FILES += \
    dlt-example-non-verbose-1.xml
