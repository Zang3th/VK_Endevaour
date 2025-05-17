#!/bin/bash

# Get all relevant paths
SCRIPT_PATH=$(dirname "$(realpath "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_PATH/..")

# Get constants
APP_SRC_PATH="$PROJECT_ROOT/Applications"
BUILD_PATH="$PROJECT_ROOT/Build/Debug/Applications"

# Applications
APP_NAME="Sandbox"
APP_EXE="$BUILD_PATH/$APP_NAME/$APP_NAME"

# Create build directory
mkdir -p "$PROJECT_ROOT/Build" && cd "$PROJECT_ROOT/Build" || exit

# Compile engine and applications
rm -f $APP_EXE
mkdir -p Debug
cd Debug
cmake ../.. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug || exit 1
mv compile_commands.json $PROJECT_ROOT
make -j || exit 1

# Count lines of code
echo -e "\nCode statistics (Lines of code):"
echo -n "Engine/Core:     "; find "$PROJECT_ROOT/Engine/Core"     -name '*.cpp' -o -name '*.hpp' | xargs cat | wc -l
echo -n "Engine/Debug:    "; find "$PROJECT_ROOT/Engine/Debug"    -name '*.cpp' -o -name '*.hpp' | xargs cat | wc -l
echo -n "Engine/Graphics: "; find "$PROJECT_ROOT/Engine/Graphics" -name '*.cpp' -o -name '*.hpp' | xargs cat | wc -l
echo -n "Engine/Vendor:   "; find "$PROJECT_ROOT/Engine/Vendor"   -name '*.cpp' -o -name '*.hpp' | xargs cat | wc -l
echo -n "Applications:    "; find "$PROJECT_ROOT/Applications"    -name '*.cpp' -o -name '*.hpp' | xargs cat | wc -l

# Copy the applications' shaders
mkdir -p "$BUILD_PATH/$APP_NAME/Shaders"
cp "$APP_SRC_PATH/$APP_NAME/Shaders"/*.spv "$BUILD_PATH/$APP_NAME/Shaders/"

# Launch app from within the buil directory
cd "$BUILD_PATH/$APP_NAME"
echo -e "\nApplication launched from:" $(pwd)
./$APP_NAME

cd $PROJECT_ROOT
