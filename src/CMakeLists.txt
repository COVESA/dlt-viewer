# Copyright (C) 2016, Jack S. Smith
# 
# This file is part of GENIVI DLT-Viewer project.
#   
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
#
# List of changes:
# 01.Oct.2016, Jack Smith <jack.smith@elektrobit.com>, Original Author
#

cmake_minimum_required (VERSION 2.8.12)
set (CMAKE_VERBOSE_MAKEFILE FALSE)


set (CMAKE_AUTOUIC ON)

# cmake 2.8.12 doesn't have AUTOUIC ??
FILE(GLOB ALL_UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)
QT5_WRAP_UI( UI_HEADERS ${ALL_UIS} )

QT5_ADD_RESOURCES(UI_RESOURCES_RCC resource.qrc)

add_executable (dlt-viewer main.cpp
    mainwindow.cpp
    project.cpp
    ecudialog.cpp
    applicationdialog.cpp
    contextdialog.cpp
    filterdialog.cpp
    plugindialog.cpp
    settingsdialog.cpp
    injectiondialog.cpp
    searchdialog.cpp
    multiplecontextdialog.cpp
    optmanager.cpp
    dltsettingsmanager.cpp
    tablemodel.cpp
    filtertreewidget.cpp
    dltfileutils.cpp
    dltfileindexer.cpp
    dlttableview.cpp
    dltexporter.cpp
    fieldnames.cpp
    dltuiutils.cpp
    workingdirectory.cpp
    jumptodialog.cpp
    searchtablemodel.cpp
    pulsebutton.cpp
    plugintreewidget.cpp
    exporterdialog.cpp
    dltmsgqueue.cpp
    dltfileindexerthread.cpp
    dltfileindexerdefaultfilterthread.cpp
    resource.qrc
    ${UI_HEADERS}
    ${UI_RESOURCES_RCC})
message( STATUS "Qt5Widgets_LIBRARIES ${Qt5Widgets_LIBRARIES}")
target_link_libraries(dlt-viewer  qdlt
                                  qextserialport
                                  ${Qt5Core_LIBRARIES}
                                  ${Qt5Network_LIBRARIES}
                                  ${Qt5Widgets_LIBRARIES}
                                  ${Qt5SerialPort_LIBRARIES})

install(TARGETS dlt-viewer DESTINATION ${EXECUTABLE_INSTALLATION_PATH})
install(DIRECTORY icon png svg DESTINATION ${RESOURCE_INSTALLATION_PATH}
			PATTERN icon/*.rc EXCLUDE )
