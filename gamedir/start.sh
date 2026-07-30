#!/bin/bash

MODE="debug"
if [ "$1" = "release" ] || [ "$1" = "debug" ]; then
    MODE="$1"
    shift
fi

TEST_NAME=""
for ((i=1; i<=$#; i++)); do
    arg="${!i}"

    if [ "$arg" = "--test" ] || [ "$arg" = "--integration-tests" ] || [ "$arg" = "--integration-test" ]; then
        next_idx=$((i+1))
        TEST_NAME="${!next_idx}"
        break
    fi
done

# SLOT01 (0) -> base
# SLOT02 (1) -> combat
# SLOT03 (2) -> party
SLOT_INDEX=-1
case "$TEST_NAME" in
    "base")  SLOT_INDEX=1 ;;
    "combat") SLOT_INDEX=2 ;;
    "party")  SLOT_INDEX=3 ;;
esac

EXTRA_ARGS=()
if [ $SLOT_INDEX -ne -1 ]; then
    EXTRA_ARGS+=("--dev-load-game=$SLOT_INDEX")
    echo "🎯 Test '$TEST_NAME' mapped to engine Save Slot $SLOT_INDEX"
fi

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

echo "Executing: ./fallout2-ce ${EXTRA_ARGS[@]} $@"
exec ./fallout2-ce "${EXTRA_ARGS[@]}" "$@"
