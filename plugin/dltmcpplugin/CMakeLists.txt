# SPDX-License-Identifier: MPL-2.0
#
# Copyright (C) 2026 DLT Viewer MCP Plugin
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

add_library(dltmcpplugin MODULE
    dltmcpplugin.cpp
    dltmcpplugin.h
    dltmcpplugin_context.cpp
    dltmcpplugin_tools_common.cpp
    dltmcpplugin_tools_common.h
    dltmcpplugin_tools_async.cpp
    dltmcpplugin_tools.cpp
    dltmcpplugin_viewer.cpp
    dltmcpserver.cpp
    dltmcpserver.h
)

target_link_libraries(dltmcpplugin qdlt ${QT_PREFIX}::Widgets ${QT_PREFIX}::Network)

add_plugin(dltmcpplugin)
