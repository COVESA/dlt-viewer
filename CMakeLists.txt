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

cmake_minimum_required (VERSION 3.1)

project (dlt-viewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set(QT_VERSION_REQ "5")

find_package(Qt5 ${QT_VERSION_REQ} REQUIRED COMPONENTS Core Network Widgets SerialPort PrintSupport)

if(Qt5Core_VERSION VERSION_LESS "5.5.1")
    # Presumably Qt5Core implies all dependent libs too
    message(FATAL_ERROR "QT minimum version required: 5.5.1")
endif()

set (CMAKE_VERBOSE_MAKEFILE FALSE)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

option(USE_QT_RPATH "Use RPATH for QT_LIBRARY_PATH to support non-standard QT install locations" ON)

if (USE_QT_RPATH)
	# Add Qt to the RPATH, so that there is no need to set LD_LIBRARY_PATH at runtime if Qt is installed in a non-standard location
	get_target_property(QT_LIBRARY_PATH Qt5::Core LOCATION)
	get_filename_component(QT_LIB_DIR ${QT_LIBRARY_PATH} DIRECTORY)
	set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${QT_LIB_DIR}")
endif()

# Injection of the GCC-specific compilation flags
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=gnu99")
    add_definitions( "-Wall" )
    add_definitions( "-Wextra" )
    add_definitions( "-pedantic" )
    add_definitions( "-Wno-variadic-macros" )
    add_definitions( "-Wno-strict-aliasing" )
endif()

# build executables in build/bin directory
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

add_definitions(-DQT5)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)   # set location for all executables in the build folder

option(USE_STANDARD_INSTALLATION_LOCATION "Use standard GNU installation locations" OFF)

if (USE_STANDARD_INSTALLATION_LOCATION)
    include(GNUInstallDirs)    # for standard installation locations
    set(PLUGIN_INSTALLATION_PATH ${CMAKE_INSTALL_FULL_LIBDIR}/dlt-viewer/plugins)
    set(RESOURCE_INSTALLATION_PATH ${CMAKE_INSTALL_FULL_DATADIR}/dlt-viewer)
    set(EXECUTABLE_INSTALLATION_PATH ${CMAKE_INSTALL_FULL_BINDIR})
    set(LIBRARY_INSTALLATION_PATH ${CMAKE_INSTALL_FULL_LIBDIR})
else()
    set(PLUGIN_INSTALLATION_PATH deploy/plugins)
    set(RESOURCE_INSTALLATION_PATH deploy)
    set(EXECUTABLE_INSTALLATION_PATH deploy)
    set(LIBRARY_INSTALLATION_PATH deploy)
endif()

add_definitions(-DPLUGIN_INSTALLATION_PATH="${PLUGIN_INSTALLATION_PATH}")

add_subdirectory(parser)
add_subdirectory(qdlt)
add_subdirectory(src)
add_subdirectory(plugin)
#add_subdirectory(dlt-console-viewer)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
