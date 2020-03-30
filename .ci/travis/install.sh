#!/bin/bash

if [[ "$(uname -s)" == 'Darwin' ]]; then
    brew link qt --force
else
    sudo apt-get update
    sudo apt-get install -y build-essential qt5-default libqt5serialport5-dev
fi
