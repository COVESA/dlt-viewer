# Helpers to detect QT version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

# Unix executable install path
target.path = $$PREFIX/usr/bin
INSTALLS += target

# Library definitions for debug and release builds
CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    QMAKE_LIBDIR += ../debug
} else {
    DESTDIR = ../release
    QMAKE_LIBDIR += ../release
}

# QT Features to be linked in
QT       += core gui
CONFIG   += console

# Detect QT5 and comply to new Widgets hierarchy
greaterThan(QT_VER_MAJ, 4) {
    QT += widgets
    INCLUDEPATH += QtWidgets
    DEFINES += QT5
}

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# Executable name
TARGET = dlt-parser

# This is an application
TEMPLATE = app

# Compile these sources
SOURCES += main.cpp\
        mainwindow.cpp \
    qdltparser.cpp

# Show these headers in the project
HEADERS  += mainwindow.h \
    version.h \
    qdltparser.h

# Compile these UI files
FORMS    += mainwindow.ui

# Related files

OTHER_FILES += \
    ../build_parser_windows_qt5.bat \
    ../README_Parser.txt \
    ../ReleaseNotes_Parser.txt
