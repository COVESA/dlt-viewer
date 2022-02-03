#!/usr/bin/env bash
set -ex

id
env
pwd

brew install qt@5
brew link qt@5 --force
# https://github.com/Homebrew/homebrew-core/issues/8392
# https://github.com/Homebrew/legacy-homebrew/issues/29938
QT_VERSION=$(brew list --versions qt@5 | sed -n '/qt@5/s/^.*[^0-9]\([0-9]*\.[0-9]*\.[0-9]*.*\).*$/\1/p')
sudo ln -s "/usr/local/Cellar/qt@5/${QT_VERSION}/mkspecs" /usr/local/mkspecs
sudo ln -s "/usr/local/Cellar/qt@5/${QT_VERSION}/plugins" /usr/local/plugins

brew install ninja tree
