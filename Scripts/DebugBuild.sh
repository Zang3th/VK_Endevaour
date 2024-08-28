#!/bin/bash

SCRIPT_DIR=$(dirname "$(realpath "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/..")
cd "$PROJECT_ROOT/Build" || exit

rm -f -r Debug/
mkdir Debug
cd Debug
cmake ../.. -D CMAKE_BUILD_TYPE=Debug -D CMAKE_CXX_FLAGS="-g"
make -j
