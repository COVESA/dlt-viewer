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


#cmake 2.8.12 doesn't have AUTOUIC ??
QT5_WRAP_UI( UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/form.ui )

add_library(dummycontrolplugin MODULE dummycontrolplugin.cpp
   form.cpp
   ${UI_HEADERS})

target_link_libraries(dummycontrolplugin qdlt Qt5::Widgets )
add_plugin(dummycontrolplugin)
