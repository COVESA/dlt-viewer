load("@bazel_skylib//rules:common_settings.bzl", "bool_flag", "string_flag")
load("//:defs.bzl", "flag_compiler_define")

string_flag(
    name = "plugin_installation_path",
    build_setting_default = "DLTViewer/usr/bin/plugins",
)

flag_compiler_define(
    name = "plugin_installation_path_define",
    flag = ":plugin_installation_path",
    visibility = [
        "//qdlt:__pkg__",
    ],
)

# 1. Define the actual boolean command-line flag
bool_flag(
    name = "DLT_USE_QT_RPATH",
    build_setting_default = False,
)

# 2. Define the matchers that select() can actually understand
config_setting(
    name = "use_qt_rpath_true",
    flag_values = {":DLT_USE_QT_RPATH": "True"},
)
