#!/bin/bash

MODE="dev"

if [ "$1" = "release" ] || [ "$1" = "debug" ] || [ "$1" = "dev" ]; then
    MODE="$1"
    shift
fi

if [ "$MODE" = "release" ]; then
    BUILD_DIR="../build-release"
    echo "🚀 Running in RELEASE mode..."
elif [ "$MODE" = "debug" ]; then
    BUILD_DIR="../build-debug"
    echo "🪲 Running in DEBUG mode with ASan..."

    export ASAN_OPTIONS="detect_leaks=1"
else
    BUILD_DIR="../build-dev"
    echo "🛠️  Running in DEV mode..."

    unset ASAN_OPTIONS
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "📂 Directory $BUILD_DIR not found. Configuring CMake..."
    cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$CMAKE_TYPE"
    if [ $? -ne 0 ]; then echo "❌ CMake configuration failed!"; exit 1; fi
fi

echo "🔨 Building project..."
cmake --build "$BUILD_DIR" -j$(nproc)
if [ $? -ne 0 ]; then
    echo "❌ Build failed! Fixing errors before running."
    exit 1
fi

cp "$BUILD_DIR/fallout2-ce/fallout2-ce" ./

echo "🎮 Executing: ./fallout2-ce $@"
exec ./fallout2-ce "$@"
