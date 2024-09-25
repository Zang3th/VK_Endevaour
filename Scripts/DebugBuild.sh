#!/bin/bash

SCRIPT_DIR=$(dirname "$(realpath "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/..")
mkdir -p "$PROJECT_ROOT/Build" && cd "$PROJECT_ROOT/Build" || exit

mkdir -p Debug
cd Debug
cmake ../.. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-glldb"
make -j

mv compile_commands.json $PROJECT_ROOT
