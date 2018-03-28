#
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

project (DLT-Viewer)

set(CMAKE_CXX_STANDARD 11)

set(QT_VERSION_REQ "5")

find_package(Qt5Core ${QT_VERSION_REQ} REQUIRED)
find_package(Qt5Quick ${QT_VERSION_REQ} REQUIRED)
find_package(Qt5Widgets ${QT_VERSION_REQ} REQUIRED)
find_package(Qt5SerialPort ${QT_VERSION_REQ} REQUIRED)

if(Qt5Core_VERSION VERSION_LESS "5.5.1")
    # Presumably Qt5Core implies all dependent libs too
    message(FATAL_ERROR "QT minimum version required: 5.5.1")
endif()

set (CMAKE_VERBOSE_MAKEFILE FALSE)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

# Add Qt to the RPATH, so that there is no need to set LD_LIBRARY_PATH at runtime if Qt is installed in a non-standard location
get_target_property(QT_LIBRARY_PATH Qt5::Core LOCATION)
get_filename_component(QT_LIB_DIR ${QT_LIBRARY_PATH} DIRECTORY)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${QT_LIB_DIR}")

add_definitions(-DQT5)

add_subdirectory(parser)
add_subdirectory(qdlt)
add_subdirectory(src)
add_subdirectory(plugin)
#add_subdirectory(dlt-console-viewer)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
