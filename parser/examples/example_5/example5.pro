# Helpers to detect QT version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

QT       += core gui network

# Detect QT5 and comply to new Widgets hierarchy
greaterThan(QT_VER_MAJ, 4) {
    QT += widgets
    INCLUDEPATH += QtWidgets
    DEFINES += QT5
}

TARGET = example_qt
TEMPLATE = app

DEFINES += DLT_EXAMPLE

INCLUDEPATH = ../src /usr/include/qdlt

SOURCES += main.cpp\
           mainwindow.cpp \
           dlt.c \
    example5.cpp

HEADERS  += mainwindow.h \
            dlt.h \
            dlt_config.h \
            dlt_protocol.h \
    example5.h \
    dlt_id.h \
    dlt_common_api.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    ../../01_Documentation/ReleaseNotes.txt \
    ../TODO
