#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
QCORO_DIR="$PROJECT_ROOT/src/vendor/qcoro"

echo "Building qCoro..."
echo "Project root: $PROJECT_ROOT"
echo "qCoro directory: $QCORO_DIR"

if [ ! -d "$QCORO_DIR" ]; then
    echo "ERROR: qCoro directory not found: $QCORO_DIR"
    exit 1
fi

cd "$QCORO_DIR"

echo "Configuring qCoro with CMake..."
cmake -B build -S . \
    -DQCORO_WITH_QTWEBSOCKETS=OFF \
    -DBUILD_TESTING=OFF \
    -DQCORO_BUILD_EXAMPLES=OFF

echo "Building qCoro..."
cmake --build build -- -j$(nproc)

echo "Installing qCoro to local install directory..."
cmake --install build --prefix install

echo ""
echo "========================================"
echo "qCoro build completed successfully!"
echo "Install directory: $QCORO_DIR/install"
echo "========================================"
