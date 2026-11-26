"""

flag_compiler_define turns a command line string flag into a compiler define

"""

load("@bazel_skylib//rules:common_settings.bzl", "BuildSettingInfo")
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")

def _flag_define_provider_impl(ctx):
    # 1. Grab the string flag value at analysis time
    flag_val = ctx.attr.flag[BuildSettingInfo].value

    # 2. Format it precisely as a C++ compiler string define
    compiler_define = 'PLUGIN_INSTALLATION_PATH=\"{}\"'.format(flag_val)

    # 3. Construct compilation context embedding our dynamic define
    compilation_context = cc_common.create_compilation_context(
        defines = depset([compiler_define]),  # Applies transitively to targets in 'deps'
    )

    return [CcInfo(compilation_context = compilation_context)]

flag_compiler_define = rule(
    implementation = _flag_define_provider_impl,
    attrs = {
        "flag": attr.label(mandatory = True, providers = [BuildSettingInfo]),
    },
    fragments = ["cpp"],
)
