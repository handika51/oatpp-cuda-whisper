#!/bin/bash
set -e

# Ensure we are in the project root
cd "$(dirname "$0")"


# Define build directory using absolute path
BASE_DIR=$(pwd)
BUILD_DIR="$BASE_DIR/build_coverage"

echo "Base directory: $(pwd)"
echo "Build directory: $(pwd)/$BUILD_DIR"

# Clean previous build if requested
if [ "$1" = "clean" ]; then
    echo "Cleaning up previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Changed to directory: $(pwd)"

# Configure CMake with Coverage Enabled
echo "Configuring CMake with Coverage..."
cmake -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug -DLCOV_PATH=/usr/bin/lcov -DGENHTML_PATH=/usr/bin/genhtml ..

# Build Tests
echo "Building Tests..."
make my-tests

# Run Tests
echo "Running Tests..."
# Allow tests to fail (exit code != 0) but continue to generate report
./my-tests || echo "Tests failed with exit code $?. Continuing to generate coverage report..."

# Generate Coverage Report
echo "Generating Coverage Report..."
make coverage_report

echo "--------------------------------------------------------"
echo "Coverage Report Generated at: $(pwd)/coverage_report/index.html"
echo "You can view it by opening the file in your browser."
echo "--------------------------------------------------------"
