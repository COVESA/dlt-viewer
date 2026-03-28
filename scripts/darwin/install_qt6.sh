#!/usr/bin/env bash

id
env
pwd

brew update
brew install qt
brew unlink qt && brew link --force qt
#brew link --overwrite qt@5 --force --overwrite
# https://github.com/Homebrew/homebrew-core/issues/8392
# https://github.com/Homebrew/legacy-homebrew/issues/29938
QT_VERSION=$(brew list --versions qt | awk '{print $2}')
echo "QT_VERSION=$QT_VERSION"
if [[ -z "$QT_VERSION" ]]; then
    echo "QT_VERSION is empty"
    exit 1
fi

# follow brew install suggestions
echo 'export PATH="/opt/homebrew/opt/qt/bin:$PATH"' >> ~/.bash_profile
export LDFLAGS="-L/opt/homebrew/opt/qt/lib"
export CPPFLAGS="-I/opt/homebrew/opt/qt/include"
export PKG_CONFIG_PATH="/opt/homebrew/opt/qt/lib/pkgconfig"
source ~/.bash_profile

echo "Run on MacOS x86"
sudo ln -s "/usr/local/Cellar/qt@5/${QT_VERSION}/mkspecs" /usr/local/mkspecs
sudo ln -s "/usr/local/Cellar/qt@5/${QT_VERSION}/plugins" /usr/local/plugins


brew install ninja tree
