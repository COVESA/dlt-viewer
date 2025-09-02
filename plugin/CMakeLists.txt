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
#

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)

function(add_plugin NAME)
    target_link_libraries(${NAME} qdlt ${QT_PREFIX}::Widgets)

    set_target_properties(${NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
        INSTALL_RPATH "$ORIGIN/../../lib;$<$<BOOL:${DLT_USE_QT_RPATH}>:${DLT_QT_LIB_DIR}>")

    install(TARGETS ${NAME}
        DESTINATION "${DLT_PLUGIN_INSTALLATION_PATH}"
        COMPONENT ${NAME})
endfunction()

add_subdirectory(dltdbusplugin)
add_subdirectory(dltlogstorageplugin)
add_subdirectory(dltsystemviewerplugin)
add_subdirectory(dlttestrobotplugin)
add_subdirectory(dltviewerplugin)
add_subdirectory(filetransferplugin)
add_subdirectory(nonverboseplugin)
add_subdirectory(dltsegmentationplugin)

option(DLT_DUMMY_PLUGINS "Build Dummy plugins" OFF)
if(DLT_DUMMY_PLUGINS)
    add_subdirectory(dummycontrolplugin)
    add_subdirectory(dummyviewerplugin)
    add_subdirectory(dummycommandplugin)
    add_subdirectory(dummydecoderplugin)
endif()
