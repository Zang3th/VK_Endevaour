#!/bin/bash

# Get all relevant paths and create build directory
SCRIPT_DIR=$(dirname "$(realpath "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/..")
mkdir -p "$PROJECT_ROOT/Build" && cd "$PROJECT_ROOT/Build" || exit
APP_DIR="$PROJECT_ROOT/Applications"
HELLO_EXE="$PROJECT_ROOT/Build/Debug/Applications/HelloTriangle/HelloTriangle"

# Compile engine and applications
rm -f $APP_EXE
mkdir -p Debug
cd Debug
cmake ../.. -DCMAKE_CXX_COMPILER=clang++-17 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-glldb"
# cmake ../.. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug
make -j
mv compile_commands.json $PROJECT_ROOT

# Launch app from within the directory
cd "$APP_DIR/HelloTriangle"
echo "Application launched from:" $(pwd)
$HELLO_EXE

cd $PROJECT_ROOT
