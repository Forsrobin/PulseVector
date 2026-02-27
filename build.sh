#!/bin/bash
set -e

# --- Default Values ---
PRESET="linux-debug"
BUILD=true
RUN=false
CLEAN=false

# --- Parse Arguments ---
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --preset) PRESET="$2"; shift ;;
        --run) RUN=true ;;
        --no-build) BUILD=false ;;
        --clean) CLEAN=true ;;
        --help) echo "Usage: ./build.sh [--preset <name>] [--run] [--no-build] [--clean]"; exit 0 ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

# --- Clean ---
if [ "$CLEAN" = true ]; then
    rm -rf build/
fi

# --- Install Dependencies ---
conan install . --output-folder=build/${PRESET} --build=missing -s build_type=$(echo ${PRESET} | cut -d'-' -f2 | sed 's/./\U&/')

# --- Configure & Build ---
if [ "$BUILD" = true ]; then
    BT=$(echo ${PRESET} | cut -d'-' -f2 | sed 's/./\U&/')
    # Check if conan toolchain exists
    TOOLCHAIN="build/${PRESET}/build/${BT}/generators/conan_toolchain.cmake"
    cmake --preset ${PRESET} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN}
    cmake --build --preset ${PRESET}
    
    # --- Run Tests ---
    echo "--- Running Tests ---"
    ctest --preset conan-debug --output-on-failure
fi

# --- Run ---
if [ "$RUN" = true ]; then
    ./build/${PRESET}/pulse-vector
fi
