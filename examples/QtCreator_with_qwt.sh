#!/bin/bash


QMAKEFEATURES="/usr/local/qwt-6.0.0/features/qwt"
echo "Set QMAKEFEATURES=$QMAKEFEATURES"
export QMAKEFEATURES


#QT_DEBUG_PLUGINS=1
#echo "Set QT_DEBUG_PLUGINS=$QT_DEBUG_PLUGINS"
#export QT_DEBUG_PLUGINS

QT_PLUGIN_PATH="/usr/local/qwt-6.0.0/plugins/"
echo "Set QT_PLUGIN_PATH=$QT_PLUGIN_PATH"
export QT_PLUGIN_PATH


LD_LIBRARY_PATH="/usr/local/qwt-6.0.0/lib/"
echo "Set LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH

echo "Start qtcreator"
qtcreator&

exit
