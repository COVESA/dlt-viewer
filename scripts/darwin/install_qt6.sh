#!/usr/bin/env bash

id
env
pwd

brew update
brew install qt@6
#brew unlink qt@6 && brew link --force qt@6

find $(brew --prefix qt@6) -name macdeployqt
ls -l $(brew --prefix qt@6)/bin

#brew link --overwrite qt@6 --force --overwrite
# https://github.com/Homebrew/homebrew-core/issues/8392
# https://github.com/Homebrew/legacy-homebrew/issues/29938
QT_VERSION=$(brew list --versions qt@6 | awk '{print $2}')

QTPATH=$(brew --prefix qt@6)
echo "QTPATH=${QTPATH}"

echo "QT_VERSION=$QT_VERSION"
if [[ -z "$QT_VERSION" ]]; then
    echo "QT_VERSION is empty"
    exit 1
fi

# follow brew install suggestions
echo 'export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"' >> ~/.bash_profile
export LDFLAGS="-L/opt/homebrew/opt/qt@6/lib"
export CPPFLAGS="-I/opt/homebrew/opt/qt@6/include"
export PKG_CONFIG_PATH="/opt/homebrew/opt/qt@6/lib/pkgconfig"
source ~/.bash_profile

uname -a
if [[ $(uname -m) == 'arm64' ]]; then
  echo "Run on MacOS arm64"
  sudo ln -s -F "/opt/homebrew/Cellar/qt@6/${QT_VERSION}/mkspecs" /usr/local/mkspecs
  sudo ln -s -F "/opt/homebrew/Cellar/qt@6/${QT_VERSION}/plugins" /usr/local/plugins
else
  echo "Run on MacOS x86"
  sudo ln -s "/usr/local/Cellar/qt@6/${QT_VERSION}/mkspecs" /usr/local/mkspecs
  sudo ln -s "/usr/local/Cellar/qt@6/${QT_VERSION}/plugins" /usr/local/plugins
fi

brew install ninja tree
