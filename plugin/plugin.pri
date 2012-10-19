# Common project definitions for a DLT Viewer plugin

# Defines
DEFINES  += QT_VIEWER
TEMPLATE  = lib
CONFIG   += plugin

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
