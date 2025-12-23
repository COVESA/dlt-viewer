# Copyright (C) 2016, Jack S. Smith
#
# This file is part of COVESA DLT-Viewer project.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.
#
# List of changes:
# 01.Oct.2016, Jack Smith <jack.smith@elektrobit.com>, Original Author

if("${QT_PREFIX}" STREQUAL "Qt6")
    QT6_ADD_RESOURCES(UI_RESOURCES_RCC resources/resource.qrc)
else()
    QT5_ADD_RESOURCES(UI_RESOURCES_RCC resources/resource.qrc)
endif()

add_executable(dlt-viewer
    main.cpp
    mainwindow.h
    mainwindow.cpp
    project.h
    project.cpp
    ecudialog.h
    ecudialog.cpp
    applicationdialog.h
    applicationdialog.cpp
    contextdialog.h
    contextdialog.cpp
    filterdialog.h
    filterdialog.cpp
    plugindialog.h
    plugindialog.cpp
    settingsdialog.h
    settingsdialog.cpp
    injectiondialog.h
    injectiondialog.cpp
    searchdialog.h
    searchdialog.cpp
    searchperformance.h
    searchperformance.cpp
    multiplecontextdialog.h
    multiplecontextdialog.cpp
    tablemodel.h
    tablemodel.cpp
    filtertreewidget.h
    filtertreewidget.cpp
    dltfileutils.h
    dltfileutils.cpp
    dltfileindexer.h
    dltfileindexer.cpp
    dlttableview.h
    dlttableview.cpp
    dltuiutils.h
    dltuiutils.cpp
    workingdirectory.h
    workingdirectory.cpp
    jumptodialog.h
    jumptodialog.cpp
    searchtablemodel.h
    searchtablemodel.cpp
    pulsebutton.h
    pulsebutton.cpp
    plugintreewidget.h
    plugintreewidget.cpp
    exporterdialog.h
    exporterdialog.cpp
    dltmsgqueue.h
    dltmsgqueue.cpp
    dltfileindexerthread.h
    dltfileindexerthread.cpp
    dltfileindexerdefaultfilterthread.h
    dltfileindexerdefaultfilterthread.cpp
    sortfilterproxymodel.h
    sortfilterproxymodel.cpp
    searchform.h
    searchform.cpp
    filtergrouplogs.h
    filtergrouplogs.cpp
    ${UI_RESOURCES_RCC}
    resources/dlt_viewer.rc
    ecutree.h
    ecutree.cpp
    fileexplorertab.ui
    fileexplorertab.h
    fileexplorertab.cpp
    applicationdialog.ui
    contextdialog.ui
    ecudialog.ui
    exporterdialog.ui
    fileexplorertab.ui
    filterdialog.ui
    injectiondialog.ui
    jumptodialog.ui
    mainwindow.ui
    multiplecontextdialog.ui
    plugindialog.ui

    searchform.ui
    settingsdialog.ui
    searchdialog.ui
)

target_link_libraries(dlt-viewer
    qdlt
    ${QT_PREFIX}::Core
    ${QT_PREFIX}::Network
    ${QT_PREFIX}::Widgets
    ${QT_PREFIX}::SerialPort)

if(CMAKE_COMPILER_IS_GNUCXX)
    # https://stackoverflow.com/questions/45329372/ubuntu-recognizes-executable-as-shared-library-and-wont-run-it-by-clicking
    # https://forum.juce.com/t/cmake-executable-build-shows-up-as-shared-library-on-linux-mint/45503/6
    target_link_options(dlt-viewer PRIVATE "-no-pie")
endif()

set_target_properties(dlt-viewer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    INSTALL_RPATH "$<$<BOOL:${LINUX}>:$ORIGIN/../lib;>$<$<BOOL:${APPLE}>:@loader_path/../Frameworks;>$<$<BOOL:${DLT_USE_QT_RPATH}>:${DLT_QT_LIB_DIR}>")

    install(TARGETS dlt-viewer
    DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}"
    # Underscore for NSIS compatibility https://gitlab.kitware.com/cmake/cmake/-/issues/19982
    COMPONENT dlt_viewer)

if(WIN32)
    set(DLT_ADDITIONAL_FILES_INSTALLATION_PATH "${DLT_EXECUTABLE_INSTALLATION_PATH}")
else()
    if(NOT DLT_ADDITIONAL_FILES_INSTALLATION_PATH)
        # Default to current directory for non-Windows systems
        # This is where additional files will be installed
        # e.g., documentation, examples, etc.
        # It can be overridden by the user via CMake options
        set(DLT_ADDITIONAL_FILES_INSTALLATION_PATH ".")
    endif()
endif()

if(DLT_INSTALL_SDK)
    install(FILES
        ../LICENSE.txt
        ../MPL.txt
        ../README.md
        ../ReleaseNotes_Viewer.txt
        DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}"
        COMPONENT dlt_viewer)
    install(FILES
        ../doc/dlt_viewer_user_manual.pdf
        ../doc/dlt_viewer_plugins_programming_guide.pdf
        DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/doc"
        COMPONENT documentation)
endif()

install(DIRECTORY
    ../filters
    DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}"
    COMPONENT dlt_viewer)
install(DIRECTORY
    ../plugin/examples
    DESTINATION "${DLT_ADDITIONAL_FILES_INSTALLATION_PATH}/plugins"
    COMPONENT dlt_viewer)

include(cmake/${CMAKE_SYSTEM_NAME}.cmake OPTIONAL)
