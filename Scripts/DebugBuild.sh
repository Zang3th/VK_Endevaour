#!/bin/bash

# Get all relevant paths
SCRIPT_DIR=$(dirname "$(realpath "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/..")

# Get constants
APP_DIR="$PROJECT_ROOT/Applications"
APP_NAME="Sandbox"
APP_PATH="$PROJECT_ROOT/Build/Debug/Applications/$APP_NAME/$APP_NAME"

# Create build directory
mkdir -p "$PROJECT_ROOT/Build" && cd "$PROJECT_ROOT/Build" || exit

# Compile engine and applications
rm -f $APP_EXE
mkdir -p Debug
cd Debug
cmake ../.. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug || exit 1
# cmake ../.. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug || exit 1
mv compile_commands.json $PROJECT_ROOT
make -j || exit 1

# Launch app from within the directory
cd "$APP_DIR/$APP_NAME"
echo "Application launched from:" $(pwd)
$APP_PATH

cd $PROJECT_ROOT
