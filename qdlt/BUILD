load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_qt//:qt.bzl", "qt_cc_library")

cc_library(
    name = "qdlt",
    srcs = [
        "qdlt.cpp",
    ],
    hdrs = [
        "qdlt.h",
    ],
    linkopts =
        select(
            {
                "@bazel_tools//src/conditions:windows": ["-defaultlib:ws2_32.lib"],
                "//conditions:default": [],
            },
        ) +
        select({
            "//:use_qt_rpath_true": [
                "-Wl,-rpath,$$ORIGIN",
                "-Wl,-rpath,/path/to/qt/libs",
            ],
            "//conditions:default": [
                "-Wl,-rpath,$$ORIGIN",
            ],
        }),
    local_defines = [
        "BYTE_ORDER=LITTLE_ENDIAN",
        "QT_VIEWER",
    ] + select({
        "@bazel_tools//src/conditions:windows": ["QDLT_LIBRARY"],
        "//conditions:default": ["_TTY_POSIX_"],
    }),
    deps = [
        ":argument",
        ":base",
        ":connection",
        ":control",
        ":control_message",
        ":decode_cache_service",
        ":default_filter",
        ":dlt_common",
        ":dlt_export_constants",
        ":dlt_protocol",
        ":dlt_types",
        ":dlt_user",
        ":exporter",
        ":field_names",
        ":file",
        ":file_projection",
        ":filter",
        ":filter_index",
        ":filter_list",
        ":importer",
        ":index_service",
        ":ip_connection",
        ":lru_cache",
        ":message",
        ":message_decoder",
        ":message_matcher",
        ":message_store",
        ":message_wrapper",
        ":opt_manager",
        ":plugin",
        ":plugin_interface",
        ":plugin_manager",
        ":search_snapshot",
        ":search_snapshot_manager",
        ":segmented_message",
        ":serial_connection",
        ":settings_manager",
        ":tcp_connection",
        ":udp_connection",
        "//:plugin_installation_path_define",
        "//src:version",
    ],
)

qt_cc_library(
    name = "field_names",
    srcs = ["fieldnames.cpp"],
    hdrs = ["fieldnames.h"],
    deps = [
        ":export_rules",
        ":settings_manager",
        "@rules_qt//:qt_core",
    ],
)

qt_cc_library(
    name = "control",
    srcs = ["qdltcontrol.cpp"],
    hdrs = ["qdltcontrol.h"],
    deps = [
        ":dlt_common",
        ":export_rules",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "importer",
    srcs = ["qdltimporter.cpp"],
    hdrs = ["qdltimporter.h"],
    deps = [
        ":dlt_common",
        ":export_rules",
        ":message",
        "@rules_qt//:qt_core",
    ],
)

qt_cc_library(
    name = "exporter",
    srcs = ["qdltexporter.cpp"],
    hdrs = ["qdltexporter.h"],
    deps = [
        "field_names",
        ":decode_cache_service",
        ":dlt_export_constants",
        ":export_rules",
        ":file",
        ":importer",
        ":message_store",
        ":opt_manager",
        ":plugin_manager",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "export_rules",
    srcs = ["export_rules.cpp"],
    hdrs = ["export_rules.h"],
    deps = [
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "export_c_rules",
    srcs = ["export_c_rules.c"],
    hdrs = ["export_c_rules.h"],
)

cc_library(
    name = "settings_manager",
    srcs = ["qdltsettingsmanager.cpp"],
    hdrs = ["qdltsettingsmanager.h"],
    deps = [
        ":export_rules",
        "//src:version",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "dlt_common",
    srcs = [
        "dlt_common.c",
        "dlt_common_cfg.c",
    ],
    hdrs = [
        "dlt_common.h",
        "dlt_common_cfg.h",
    ],
    deps = [
        ":dlt_protocol",
        ":dlt_types",
        ":export_c_rules",
        "//src:version",
    ],
)

cc_library(
    name = "dlt_user",
    srcs = ["dlt_user.c"],
    hdrs = ["dlt_user.h"],
    deps = [
        ":dlt_common",
    ],
)

cc_library(
    name = "dlt_types",
    srcs = ["dlt_types.c"],
    hdrs = ["dlt_types.h"],
)

cc_library(
    name = "dlt_protocol",
    srcs = ["dlt_protocol.c"],
    hdrs = ["dlt_protocol.h"],
)

cc_library(
    name = "base",
    srcs = ["qdltbase.cpp"],
    hdrs = ["qdltbase.h"],
    deps = [
        ":export_rules",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "argument",
    srcs = ["qdltargument.cpp"],
    hdrs = ["qdltargument.h"],
    deps = [
        ":base",
        ":dlt_common",
        ":export_rules",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "message",
    srcs = ["qdltmsg.cpp"],
    hdrs = ["qdltmsg.h"],
    deps = [
        ":argument",
        ":base",
        ":dlt_common",
        ":export_rules",
    ],
)

cc_library(
    name = "message_wrapper",
    srcs = ["qdltmsgwrapper.cpp"],
    hdrs = ["qdltmsgwrapper.h"],
    deps = [
        ":dlt_common",
        ":export_rules",
    ],
)

cc_library(
    name = "segmented_message",
    srcs = ["qdltsegmentedmsg.cpp"],
    hdrs = ["qdltsegmentedmsg.h"],
    deps = [
        ":export_rules",
        ":message",
    ],
)

cc_library(
    name = "filter",
    srcs = ["qdltfilter.cpp"],
    hdrs = ["qdltfilter.h"],
    deps = [
        ":dlt_common",
        ":export_rules",
        ":message",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "filter_list",
    srcs = ["qdltfilterlist.cpp"],
    hdrs = ["qdltfilterlist.h"],
    deps = [
        ":export_rules",
        ":filter",
        ":message",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "file",
    srcs = [
        "qdltfile.cpp",
    ],
    hdrs = [
        "qdltfile.h",
    ],
    deps = [
        ":dlt_common",
        ":export_rules",
        ":filter",
        ":filter_list",
        ":message",
        ":search_snapshot",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "search_snapshot",
    srcs = ["searchsnapshot.cpp"],
    hdrs = ["searchsnapshot.h"],
    deps = [
        ":export_rules",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "search_snapshot_manager",
    srcs = ["search_snapshot_manager.cpp"],
    hdrs = ["search_snapshot_manager.h"],
    deps = [
        ":export_rules",
        ":file",
        ":search_snapshot",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "control_message",
    srcs = ["qdltctrlmsg.cpp"],
    hdrs = ["qdltctrlmsg.h"],
    deps = [
        ":dlt_common",
        ":export_rules",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "message_store",
    srcs = ["messagestore.cpp"],
    hdrs = ["messagestore.h"],
    deps = [
        ":export_rules",
        ":file",
    ],
)

cc_library(
    name = "message_matcher",
    srcs = [
        "dltmessagematcher.cpp",
    ],
    hdrs = [
        "dltmessagematcher.h",
    ],
    deps = [
        ":export_rules",
        ":message",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "index_service",
    srcs = ["indexservice.cpp"],
    hdrs = ["indexservice.h"],
    deps = [
        ":export_rules",
    ],
)

cc_library(
    name = "message_decoder",
    srcs = ["qdltmessagedecoder.cpp"],
    hdrs = ["qdltmessagedecoder.h"],
    deps = [
        ":export_rules",
        ":message",
    ],
)

cc_library(
    name = "connection",
    srcs = [
        "qdltconnection.cpp",
    ],
    hdrs = [
        "qdltconnection.h",
    ],
    deps = [
        ":dlt_common",
        ":export_rules",
        ":message",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "plugin_interface",
    srcs = ["plugininterface.cpp"],
    hdrs = ["plugininterface.h"],
    deps = [
        ":connection",
        ":control",
        ":file",
        ":message",
        ":message_decoder",
        "@rules_qt//:qt_core",
        "@rules_qt//:qt_widgets",
    ],
)

cc_library(
    name = "plugin",
    srcs = ["qdltplugin.cpp"],
    hdrs = ["qdltplugin.h"],
    deps = [
        ":export_rules",
        ":message",
        ":plugin_interface",
        "@rules_qt//:qt_core",
        "@rules_qt//:qt_widgets",
    ],
)

cc_library(
    name = "plugin_manager",
    srcs = ["qdltpluginmanager.cpp"],
    hdrs = ["qdltpluginmanager.h"],
    deps = [
        ":connection",
        ":control",
        ":export_rules",
        ":plugin",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "decode_cache_service",
    srcs = ["decodecacheservice.cpp"],
    hdrs = ["decodecacheservice.h"],
    deps = [
        ":export_rules",
        ":file",
        ":message",
        ":message_store",
        ":plugin_manager",
    ],
)

cc_library(
    name = "default_filter",
    srcs = ["qdltdefaultfilter.cpp"],
    hdrs = ["qdltdefaultfilter.h"],
    deps = [
        ":export_rules",
        ":filter_index",
        ":filter_list",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "filter_index",
    srcs = ["qdltfilterindex.cpp"],
    hdrs = ["qdltfilterindex.h"],
    deps = [
        ":dlt_common",
        ":export_rules",
        ":filter_list",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "file_projection",
    srcs = ["qdltfileprojection.cpp"],
    hdrs = ["qdltfileprojection.h"],
    deps = [
        ":export_rules",
        ":file",
    ],
)

cc_library(
    name = "opt_manager",
    srcs = ["qdltoptmanager.cpp"],
    hdrs = ["qdltoptmanager.h"],
    deps = [
        ":dlt_export_constants",
        ":export_rules",
        "//src:version",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "dlt_export_constants",
    srcs = ["dlt_export_constants.cpp"],
    hdrs = ["dlt_export_constants.h"],
)

cc_library(
    name = "lru_cache",
    srcs = ["qdltlrucache.cpp"],
    hdrs = ["qdltlrucache.h"],
)

cc_library(
    name = "ip_connection",
    srcs = ["qdltipconnection.cpp"],
    hdrs = ["qdltipconnection.h"],
    deps = [
        ":connection",
        ":dlt_common",
        ":export_rules",
        "@rules_qt//:qt_core",
    ],
)

cc_library(
    name = "serial_connection",
    srcs = ["qdltserialconnection.cpp"],
    hdrs = ["qdltserialconnection.h"],
    deps = [
        ":connection",
        ":dlt_common",
        ":export_rules",
        "@rules_qt//:qt_core",
        "@rules_qt//:qt_serial_port",
    ],
)

cc_library(
    name = "tcp_connection",
    srcs = ["qdlttcpconnection.cpp"],
    hdrs = ["qdlttcpconnection.h"],
    deps = [
        ":dlt_common",
        ":export_rules",
        ":ip_connection",
        "@rules_qt//:qt_core",
        "@rules_qt//:qt_network",
    ],
)

cc_library(
    name = "udp_connection",
    srcs = ["qdltudpconnection.cpp"],
    hdrs = ["qdltudpconnection.h"],
    deps = [
        ":dlt_common",
        ":export_rules",
        ":ip_connection",
        "@rules_qt//:qt_core",
        "@rules_qt//:qt_network",
    ],
)
