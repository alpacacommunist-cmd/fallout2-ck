#!/bin/bash

MODE="${1:-debug}"
shift

if [ "$MODE" = "release" ]; then
    BUILD_DIR="../build-release"
    echo "🚀 Running in RELEASE mode..."
else
    BUILD_DIR="../build-debug"
    echo "🪲 Running in DEBUG mode with ASan..."
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Error: Directory $BUILD_DIR does not exist!"
    exit 1
fi

cp "$BUILD_DIR/fallout2-ce/fallout2-ce" ./
export ASAN_OPTIONS="detect_leaks=1"

echo "🎮 Executing: ./fallout2-ce $@"
exec ./fallout2-ce "$@"
