#!/bin/bash

# Release packaging script for raycasting-engine
# Creates a distributable package with the game and assets

set -e

echo "=== Raycasting Engine - Release Packaging ==="

# Configuration
PROJECT_NAME="raycasting-engine"
BUILD_DIR="out/Release"
PACKAGE_DIR="dist"
ASSETS_DIR="ressources"

# Clean previous package
echo "Cleaning previous packages..."
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/$PROJECT_NAME"

# Check if release build exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Release build not found!"
    echo "Please run ./scripts/build-release.sh first"
    exit 1
fi

# Copy executable
echo "Copying executable..."
if [ -f "$BUILD_DIR/$PROJECT_NAME" ]; then
    cp "$BUILD_DIR/$PROJECT_NAME" "$PACKAGE_DIR/$PROJECT_NAME/"
elif [ -f "$BUILD_DIR/$PROJECT_NAME.exe" ]; then
    cp "$BUILD_DIR/$PROJECT_NAME.exe" "$PACKAGE_DIR/$PROJECT_NAME/"
else
    echo "Error: Executable not found in $BUILD_DIR"
    exit 1
fi

# Copy assets
echo "Bundling assets..."
if [ -d "$ASSETS_DIR" ]; then
    cp -r "$ASSETS_DIR" "$PACKAGE_DIR/$PROJECT_NAME/"
else
    echo "Warning: Assets directory not found, creating empty directory"
    mkdir -p "$PACKAGE_DIR/$PROJECT_NAME/$ASSETS_DIR"
fi

# Copy README and LICENSE
echo "Copying documentation..."
[ -f "README.md" ] && cp "README.md" "$PACKAGE_DIR/$PROJECT_NAME/"
[ -f "LICENSE" ] && cp "LICENSE" "$PACKAGE_DIR/$PROJECT_NAME/"

# Create archive
echo "Creating release archive..."
cd "$PACKAGE_DIR"
if command -v zip &> /dev/null; then
    zip -r "$PROJECT_NAME-release.zip" "$PROJECT_NAME"
    echo "Created: $PACKAGE_DIR/$PROJECT_NAME-release.zip"
elif command -v tar &> /dev/null; then
    tar -czf "$PROJECT_NAME-release.tar.gz" "$PROJECT_NAME"
    echo "Created: $PACKAGE_DIR/$PROJECT_NAME-release.tar.gz"
else
    echo "Warning: No archiving tool found (zip or tar)"
fi
cd ..

echo ""
echo "=== Packaging Complete ==="
echo "Release package is in: $PACKAGE_DIR/"
