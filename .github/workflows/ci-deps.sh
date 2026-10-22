#!/bin/bash

set -o nounset
set -o errexit

case $1 in
    macos-*)
        brew update
        brew install ninja sfiera/gn/gn
        pip3 install --break-system-packages --user pytest pygments
        ;;

    ubuntu-*)
        sudo apt-get update
        sudo apt-get install -y --no-install-recommends python3 python3-pip build-essential clang make generate-ninja ninja-build
        pip3 install --user pytest pygments
        ;;

    windows-*)
        curl -Lo gn.zip https://chrome-infra-packages.appspot.com/dl/gn/gn/windows-amd64/+/latest
        unzip gn.zip gn.exe
        curl -Lo ninja.zip https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-win.zip
        unzip ninja.zip ninja.exe
        ;;

    *)
        echo >&2 "$1: unknown runner os"
        exit 1
        ;;
esac
