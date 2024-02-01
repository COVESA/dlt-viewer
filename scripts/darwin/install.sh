#!/usr/bin/env bash
set -ex

id
env
pwd

brew install qt@6
brew link qt@6 --force
# https://github.com/Homebrew/homebrew-core/issues/8392
# https://github.com/Homebrew/legacy-homebrew/issues/29938
QT_VERSION=$(brew list --versions qt@6 | awk '{print $2}')
echo "QT_VERSION=$QT_VERSION"
if [[ -z "$QT_VERSION" ]]; then
    echo "QT_VERSION is empty"
    exit 1
fi

if [[ $(uname -m) == 'arm64' ]]; then
  sudo ln -s -F "/opt/homebrew/Cellar/qt/${QT_VERSION}/mkspecs" /usr/local/mkspecs
  sudo ln -s -F "/opt/homebrew/Cellar/qt/${QT_VERSION}/plugins" /usr/local/plugins
else
  sudo ln -s "/usr/local/Cellar/qt@6/${QT_VERSION}/mkspecs" /usr/local/mkspecs
  sudo ln -s "/usr/local/Cellar/qt@6/${QT_VERSION}/plugins" /usr/local/plugins
fi

brew install ninja tree
