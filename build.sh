#!/bin/bash

#
# MIT License
#
# Copyright (c)  "2025" Talik A. Kasozi
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Exit immediately if a command exits with a non-zero status.
set -e

export PATH="/c/msys64/mingw64/bin:/c/msys64/usr/bin:/c/Program Files (x86)/Inno Setup 6:/c/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/9.0.7/runtimes/win-x64/native:$PATH"

# --- Configuration ---
# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Assume the CMakeLists.txt is in the same directory as this script (project root)
# If your CMakeLists.txt is elsewhere, adjust this path accordingly.
SOURCE_DIR="${SCRIPT_DIR}"

# Default build directory name (will be created inside SOURCE_DIR)
BUILD_SUBDIR="build"
BUILD_DIR="${SOURCE_DIR}/${BUILD_SUBDIR}"

# Default build type
BUILD_TYPE="Release"

# Options
CLEAN_BUILD=false
NUM_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1) # Number of parallel jobs

# --- Main Script Logic ---

echo "----------------------------------------------------"
echo "Starting CMake Project Build"
echo "----------------------------------------------------"
echo "Source Directory:   ${SOURCE_DIR}"
echo "Build Subdirectory: ${BUILD_SUBDIR}"
echo "Full Build Path:    ${BUILD_DIR}"
echo "Build Type:         ${BUILD_TYPE}"
echo "Clean Build:        ${CLEAN_BUILD}"
echo "Parallel Jobs:      ${NUM_JOBS}"
echo "----------------------------------------------------"

# 1. Create build directory (if it doesn't exist)
echo ""
echo "--- Ensuring build directory exists: ${BUILD_DIR} ---"
mkdir -p "${BUILD_DIR}"
echo "Build directory ready."

# 3. Configure the project using CMake
# We run cmake from any directory, specifying source and build dirs explicitly.
echo ""
echo "--- Configuring CMake project (Build Type: ${BUILD_TYPE}) ---"

if [ ! -d "${VCPKG_ROOT}" ]; then
    echo "Error: Vcpkg root directory not found at ${VCPKG_ROOT}."
    exit 1
fi

VCPKG_CMAKE_PATH="$("${VCPKG_ROOT}/vcpkg.exe" fetch cmake | tail -n 1)"
VCPKG_NINJA_PATH="$("${VCPKG_ROOT}/vcpkg.exe" fetch ninja | tail -n 1)"
DOTNET_PATH="$(which "dotnet.exe" | tail -n 1)"

if [ -z "$VCPKG_CMAKE_PATH" ] || [ -z "$VCPKG_NINJA_PATH" ] || [ -z "$DOTNET_PATH" ]; then
    echo "Error: Could not find Vcpkg CMake dotnet or Ninja executables."
    exit 1
fi

if [ -z "$QT_PATH" ]; then
    echo "Error: QT_PATH environment variable is not set."
    echo "Please set it to your Qt installation directory (e.g., C:/Qt/6.8.3/mingw_64)."
    exit 1
fi

echo "Found Vcpkg CMake executable at: $VCPKG_CMAKE_PATH"
echo "Found Vcpkg Ninja executable at: $VCPKG_NINJA_PATH"
echo "Found dotnet installation at: $DOTNET_PATH"

CMAKE_BIN_DIR=$(dirname "$VCPKG_CMAKE_PATH")
NINJA_BIN_DIR=$(dirname "$VCPKG_NINJA_PATH")
DOTNET_BIN_DIR=$(dirname "$DOTNET_PATH")

# Re set the PATH to include the MinGW compiler binaries. 
# For some reason, the PATH is not set correctly to include Mingw in the base image.
export PATH="${CMAKE_BIN_DIR}:${NINJA_BIN_DIR}:${DOTNET_BIN_DIR}:${PATH}"

echo "Env PATH: ${PATH}"

export VCPKG_TARGET_TRIPLET="x64-mingw-dynamic"
export  VCPKG_DEFAULT_HOST_TRIPLET="x64-mingw-dynamic"

echo "--- Set VCPKG_TARGET_TRIPLET to: ${VCPKG_TARGET_TRIPLET} ---"

# Install the vcpkg.json file. Should resolve dependencies and install them if needed.
# "${VCPKG_ROOT}/vcpkg.exe" install --triplet=${VCPKG_TARGET_TRIPLET} --host-triplet=${VCPKG_TARGET_TRIPLET} --allow-unsupported

ls -al "${DOTNET_BIN_DIR}"

# The -S option specifies the source directory.
# The -B option specifies the build directory (created if it doesn't exist).
"$VCPKG_CMAKE_PATH" -B build \
    -S . \
    -G "Ninja" \
    -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="${VCPKG_TARGET_TRIPLET}" \
    -DVCPKG_DEFAULT_HOST_TRIPLET="${VCPKG_TARGET_TRIPLET}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH="${QT_PATH}" \
    -DCMAKE_DOTNET_TARGET_FRAMEWORK="/c/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/9.0.7/runtimes/win-x64/native" \
    -DCMAKE_MAKE_PROGRAM="${VCPKG_NINJA_PATH}"

echo "CMake configuration complete."

# 4. Build the project
echo ""
echo "--- Building project using CMake (Jobs: ${NUM_JOBS}) ---"
# The --build option tells CMake to drive the build process.
# --config is mostly for multi-configuration generators (like Visual Studio).
# For single-config (Makefiles, Ninja), CMAKE_BUILD_TYPE is used at configure time.
# The -- -j${NUM_JOBS} part passes the parallel job count to the underlying build tool (make/ninja).
"$VCPKG_CMAKE_PATH" --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -- -j"${NUM_JOBS}"

echo "Build complete."

echo ""
echo "----------------------------------------------------"
echo "Build process finished successfully!"
echo "Build artifacts are in: ${BUILD_DIR}"
echo "----------------------------------------------------"

echo "--- Packaging with Inno Setup ---"
