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

# build plugins in build/bin/plugins directory
set(PLUGIN_DIR ${BIN_DIR}/plugins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PLUGIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_DIR})


function(add_plugin NAME)

    target_link_libraries(${NAME} qdlt Qt5::Widgets)
    install(TARGETS ${NAME} DESTINATION ${PLUGIN_INSTALLATION_PATH})

    # set the location of the plugins in such a way that they can be located by the dlt-viewer executable when running in
    # directly from the build folder (without "make install")
    set_target_properties(${NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

endfunction()


add_subdirectory(dltviewerplugin)
add_subdirectory(nonverboseplugin)
add_subdirectory(filetransferplugin)
add_subdirectory(dltsystemviewerplugin)
add_subdirectory(dummycontrolplugin)
add_subdirectory(dummyviewerplugin)
add_subdirectory(dummycommandplugin)
add_subdirectory(dummydecoderplugin)
add_subdirectory(dltdbusplugin)
add_subdirectory(dltlogstorageplugin)
