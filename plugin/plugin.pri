# Common project definitions for a DLT Viewer plugin

# Helpers to detect QT version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

# Uncomment to add debug symbols to Release build
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g
#QMAKE_LFLAGS_RELEASE =

# Defines
DEFINES  += QT_VIEWER

# This is a library
TEMPLATE  = lib

# ...of type plugin
CONFIG   += plugin

# Used QT features
QT += core gui

# Detect QT5 and comply to new Widget hierarchy
greaterThan(QT_VER_MAJ, 4) {
    QT += widgets
    INCLUDEPATH += QtWidgets
    DEFINES += QT5
}


# Include path
INCLUDEPATH += ../../src \
            ../../qdlt

# Library path
CONFIG(debug, debug|release) {
    QMAKE_LIBDIR += ../../debug
}
else {
    QMAKE_LIBDIR += ../../release
}

# Libraries
CONFIG(debug, debug|release) {
    LIBS += -lqdltd
}
else {
    LIBS += -lqdlt
}

# Destination
CONFIG(debug, debug|release) {
    DESTDIR = ../../debug/plugins
}
else {
    DESTDIR = ../../release/plugins
}
target.path = /usr/share/dlt-viewer/plugins
INSTALLS += target
