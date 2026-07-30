#!/bin/bash

MODE="debug"
if [ "$1" = "release" ] || [ "$1" = "debug" ]; then
    MODE="$1"
    shift
fi

TEST_NAME="$1"

if [ -z "$TEST_NAME" ]; then
    echo "❌ Error: Please specify a test name! Example: ./test.sh init"
    exit 1
fi

SLOT_INDEX=-1
case "$TEST_NAME" in
    "init"|"base")   SLOT_INDEX=1 ;;
    "combat")        SLOT_INDEX=2 ;;
    "party")         SLOT_INDEX=3 ;;
esac

if [ $SLOT_INDEX -eq -1 ]; then
    echo "❌ Error: Unknown test suite '$TEST_NAME'!"
    exit 1
fi

if [ "$MODE" = "release" ]; then
    BUILD_DIR="../build-release"
    echo "🚀 [TEST] Running on RELEASE binary..."
else
    BUILD_DIR="../build-debug"
    echo "🪲 [TEST] Running on DEBUG binary with ASan..."
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Error: Directory $BUILD_DIR does not exist!"
    exit 1
fi

cp "$BUILD_DIR/fallout2-ce/fallout2-ce" ./
export ASAN_OPTIONS="detect_leaks=1"

echo "📦 Preparing clean test environment for suite: '$TEST_NAME'..."

if [ -f "mods.lua" ]; then
    mv "mods.lua" "mods.lua_backup_temp"
fi
if [ -f "../tests_env/mods.lua" ]; then
    cp "../tests_env/mods.lua" "mods.lua"
fi

if [ -d "../mods" ]; then
    mv "../mods" "../mods_backup_temp"
fi
if [ -d "../tests_env/mods" ]; then
    cp -r "../tests_env/mods" "../mods"
else
    mkdir "../mods"
fi

if [ -d "data/SAVEGAME" ]; then
    mv "data/SAVEGAME" "data/SAVEGAME_backup_temp"
fi
if [ -d "../tests_env/SAVEGAME" ]; then
    cp -r "../tests_env/SAVEGAME" "data/SAVEGAME"
else
    mkdir -p "data/SAVEGAME"
fi

echo "🎯 Executing automated test suite..."
./fallout2-ce --dev-load-game=$SLOT_INDEX --test "$TEST_NAME"
GAME_EXIT_CODE=$?

echo "🧹 Cleaning up test environment and restoring your workspace..."

rm -f "mods.lua"
rm -rf "../mods"
rm -rf "data/SAVEGAME"

if [ -f "mods.lua_backup_temp" ]; then
    mv "mods.lua_backup_temp" "mods.lua"
fi

if [ -d "../mods_backup_temp" ]; then
    mv "../mods_backup_temp" "../mods"
fi

if [ -d "data/SAVEGAME_backup_temp" ]; then
    mv "data/SAVEGAME_backup_temp" "data/SAVEGAME"
fi

if [ $GAME_EXIT_CODE -eq 0 ]; then
    echo "✅ TEST '$TEST_NAME' PASSED SUCCESSFULLY!"
else
    echo "❌ TEST '$TEST_NAME' FAILED! Check logs above."
fi

exit $GAME_EXIT_CODE
