#!/usr/bin/env bash

id
env
pwd

brew update
brew install qt@5
brew unlink qt@5 && brew link --force qt@5
#brew link --overwrite qt@5 --force --overwrite
# https://github.com/Homebrew/homebrew-core/issues/8392
# https://github.com/Homebrew/legacy-homebrew/issues/29938
QT_VERSION=$(brew list --versions qt@5 | awk '{print $2}')
echo "QT_VERSION=$QT_VERSION"
if [[ -z "$QT_VERSION" ]]; then
    echo "QT_VERSION is empty"
    exit 1
fi

# follow brew install suggestions
echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.bash_profile
export LDFLAGS="-L/opt/homebrew/opt/qt@5/lib"
export CPPFLAGS="-I/opt/homebrew/opt/qt@5/include"
export PKG_CONFIG_PATH="/opt/homebrew/opt/qt@5/lib/pkgconfig"
source ~/.bash_profile

uname -a
if [[ $(uname -m) == 'arm64' ]]; then
  echo "Run on MacOS arm64"
  sudo ln -s -F "/opt/homebrew/Cellar/qt@5/${QT_VERSION}/mkspecs" /usr/local/mkspecs
  sudo ln -s -F "/opt/homebrew/Cellar/qt@5/${QT_VERSION}/plugins" /usr/local/plugins
else
  echo "Run on MacOS x86"
  sudo ln -s "/usr/local/Cellar/qt@5/${QT_VERSION}/mkspecs" /usr/local/mkspecs
  sudo ln -s "/usr/local/Cellar/qt@5/${QT_VERSION}/plugins" /usr/local/plugins
fi

brew install ninja tree
