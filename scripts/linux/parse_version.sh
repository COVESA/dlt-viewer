#!/usr/bin/env bash
set -eu

_print_usage() {
    echo "Parse DLT Viewer version.h and return version number depending on requested part."
    echo "Usage: parse_version.sh <path to version.h> <PACKAGE_MAJOR_VERSION | PACKAGE_MINOR_VERSION | PACKAGE_PATCH_LEVEL>"
}

if [[ $# -ne 2 ]]; then
    _print_usage
    exit 1
fi

if [ "${2}" != "PACKAGE_MAJOR_VERSION" -a "${2}" != "PACKAGE_MINOR_VERSION" -a "${2}" != "PACKAGE_PATCH_LEVEL" ]; then
    _print_usage
    exit 1
fi

echo $(grep ${2} ${1} | awk '{print $3}')
