#!/bin/bash

SCRIPT_DIR=$(dirname "$(realpath "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/..")

cppcheck $PROJECT_ROOT -i Build/Debug -i Engine/Vendor --check-level=exhaustive
