# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

greaterThan(QT_VER_MAJ, 4) {
    QT += printsupport
}

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# target name
TARGET = $$qtLibraryTarget(filetransferplugin)

# plugin header files
HEADERS += \
    filetransferplugin.h \
    form.h \
    file.h \
    imagepreviewdialog.h \
    textviewdialog.h \
    globals.h \
    configuration.h

# plugin source files
SOURCES += \
    filetransferplugin.cpp \
    form.cpp \
    file.cpp \
    imagepreviewdialog.cpp \
    textviewdialog.cpp \
    configuration.cpp

# plugin forms
FORMS += \
    form.ui \
    imagepreviewdialog.ui \
    textviewdialog.ui

