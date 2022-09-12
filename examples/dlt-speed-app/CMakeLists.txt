#######
# Dlt Speed App - Example Spped Application
# @licence make begin@
#
# Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.aw.wenzel@bmw.de>
# 
# Copyright (C) 2011-2012, BMW AG - Alexander Wenzel <alexander.aw.wenzel@bmw.de>
#
# Contributions are licensed to the COVESA Alliance under one or more
# Contribution License Agreements.
# 
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# @licence end@
########


# Minimum cmake version required
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)


# Name for the entire project
PROJECT(dlt-speed-app)


# Set version parameters
SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 2 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 0 )
SET( ${PROJECT_NAME}_VERSION_STATE "RELEASE" )
SET( PRINT_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL})
SET( PRINT_VERSION_STATE ${${PROJECT_NAME}_VERSION_STATE})

# Print version information
MESSAGE("VERSION ${PRINT_VERSION}")
MESSAGE("VERSION_STATE ${PRINT_VERSION_STATE}")



# Use PkgConfig to check for automtoive-dlt
FIND_PACKAGE(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)
  

# Set CMAKE variables to given values
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "-std=gnu++0x")
endif()
SET(CMAKE_CXX_FLAGS "-Wall")
SET(CMAKE_CXX_FLAGS "-Wextra")
#SET(CMAKE_CXX_FLAGS "-pedantic")
#SET(CMAKE_LD_FLAGS "-ldlt")
SET(CMAKE_BUILD_TYPE Release)



# Add include directories to the build
INCLUDE_DIRECTORIES(
  src
  ${GLIB_INCLUDE_DIRS}
  ${DLT_INCLUDE_DIRS}
)


# Specify directories in which the linker will look for libraries
LINK_DIRECTORIES(
    ${LINK_DIRECTORIES}
    ${DLT_LIBRARY_DIRS}
)


# GLOB will generate a list of all files that match the globbing expressions and store it into "SOURCES"
FILE(GLOB SOURCES
   "*.c"
)

# Add an executable to the project using the specified source files
ADD_EXECUTABLE(
   dlt-speed-app
    ${SOURCES}
)


# Link a target to given libraries
TARGET_LINK_LIBRARIES(
   dlt-speed-app
   ${GLIB_LIBRARIES}
   ${DLT_LIBRARIES}
)


# Installing files
INSTALL(TARGETS dlt-speed-app
  RUNTIME DESTINATION bin
)
