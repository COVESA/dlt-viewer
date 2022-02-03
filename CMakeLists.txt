# SPDX-License-Identifier: MPL-2.0
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

cmake_minimum_required (VERSION 3.15)

include(scripts/windows/version.cmake)
include(scripts/linux/version.cmake)

project(dlt-viewer
    VERSION ${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}
    DESCRIPTION "DLT Viewer")
message(STATUS "DLT Viewer version: ${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}-${DLT_VERSION_SUFFIX}")

file(WRITE "${CMAKE_BINARY_DIR}/version.txt" "${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}")
file(WRITE "${CMAKE_BINARY_DIR}/full_version.txt" "${DLT_PROJECT_VERSION_MAJOR}.${DLT_PROJECT_VERSION_MINOR}.${DLT_PROJECT_VERSION_PATCH}-${DLT_VERSION_SUFFIX}")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

find_package(Qt5 "5" REQUIRED COMPONENTS Core Network Widgets SerialPort PrintSupport)

if(Qt5Core_VERSION VERSION_LESS "5.5.1")
    # Presumably Qt5Core implies all dependent libs too
    message(FATAL_ERROR "QT minimum version required: 5.5.1")
endif()

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

get_target_property(DLT_QT5_LIBRARY_PATH Qt5::Core LOCATION)
get_filename_component(DLT_QT5_LIB_DIR ${DLT_QT5_LIBRARY_PATH} DIRECTORY)

# Add Qt to the RPATH, so that there is no need to set LD_LIBRARY_PATH at runtime if Qt is installed in a non-standard location
option(DLT_USE_QT_RPATH "Use RPATH for QT_LIBRARY_PATH to support non-standard QT install locations" ON)

# Injection of the GCC-specific compilation flags
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
    add_definitions("-Wall" "-Wextra" "-pedantic" "-Wno-variadic-macros" "-Wno-strict-aliasing" "-fPIC")
endif()

add_compile_definitions(QT5)

option(DLT_USE_STANDARD_INSTALLATION_LOCATION "Use standard GNU installation locations" OFF)
if(NOT WIN32)
    if(NOT DLT_APP_DIR_NAME)
        set(DLT_APP_DIR_NAME "DLTViewer")
        if(APPLE)
            set(DLT_APP_DIR_NAME "DLTViewer.app")
        endif()
    endif()
    if(DLT_USE_STANDARD_INSTALLATION_LOCATION)
        include(GNUInstallDirs)
        set(DLT_PLUGIN_INSTALLATION_PATH "${CMAKE_INSTALL_FULL_LIBDIR}/dlt-viewer/plugins")
        set(DLT_RESOURCE_INSTALLATION_PATH "${CMAKE_INSTALL_FULL_DATADIR}/dlt-viewer")
        set(DLT_EXECUTABLE_INSTALLATION_PATH "${CMAKE_INSTALL_FULL_BINDIR}")
        set(DLT_LIBRARY_INSTALLATION_PATH "${CMAKE_INSTALL_FULL_LIBDIR}")
    else()
        if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
            set(CMAKE_INSTALL_PREFIX "DLTViewer")
        endif()
        if(NOT DLT_PLUGIN_INSTALLATION_PATH)
            set(DLT_PLUGIN_INSTALLATION_PATH "${DLT_APP_DIR_NAME}/usr/bin/plugins")
        endif()
        if(NOT DLT_RESOURCE_INSTALLATION_PATH)
            set(DLT_RESOURCE_INSTALLATION_PATH "${DLT_APP_DIR_NAME}/usr/share")
        endif()
        if(NOT DLT_EXECUTABLE_INSTALLATION_PATH)
            set(DLT_EXECUTABLE_INSTALLATION_PATH "${DLT_APP_DIR_NAME}/usr/bin")
        endif()
        if(NOT DLT_LIBRARY_INSTALLATION_PATH)
            set(DLT_LIBRARY_INSTALLATION_PATH "${DLT_APP_DIR_NAME}/usr/lib")
        endif()
    endif()
else()
    if(NOT DLT_APP_DIR_NAME)
        set(DLT_APP_DIR_NAME "./")
    endif()
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        # AppData is a modern install location. No Admin rights required.
        file(TO_CMAKE_PATH "$ENV{LOCALAPPDATA}/Programs/dlt-viewer" CMAKE_INSTALL_PREFIX)
        set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "..." FORCE)
    endif()

    if(NOT DLT_PLUGIN_INSTALLATION_PATH)
        set(DLT_PLUGIN_INSTALLATION_PATH "plugins")
    endif()
    if(NOT DLT_RESOURCE_INSTALLATION_PATH)
        set(DLT_RESOURCE_INSTALLATION_PATH ".")
    endif()
    if(NOT DLT_EXECUTABLE_INSTALLATION_PATH)
        set(DLT_EXECUTABLE_INSTALLATION_PATH ".")
    endif()
    if(NOT DLT_LIBRARY_INSTALLATION_PATH)
        set(DLT_LIBRARY_INSTALLATION_PATH ".")
    endif()
endif()

option(DLT_PARSER "Build DLT Parser" OFF)

if(DLT_PARSER)
    add_subdirectory(parser)
endif()

add_subdirectory(qdlt)
add_subdirectory(src)
add_subdirectory(plugin)

message(STATUS "\n\t** DLT Viewer Build Summary **")
message(STATUS "\tCMAKE_INSTALL_PREFIX:         ${CMAKE_INSTALL_PREFIX}")
message(STATUS "\tCMAKE_BUILD_TYPE:             ${CMAKE_BUILD_TYPE}")
message(STATUS "\tCMAKE_SYSTEM_PROCESSOR:       ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "\tCMAKE_SYSTEM_NAME:            ${CMAKE_SYSTEM_NAME}")
message(STATUS "\tDLT_QT5_LIB_DIR:               ${DLT_QT5_LIB_DIR}\n")

# Must be last
include(scripts/windows/package.cmake)
include(scripts/linux/package.cmake)
include(scripts/darwin/package.cmake)
