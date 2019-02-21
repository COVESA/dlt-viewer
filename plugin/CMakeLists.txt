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
