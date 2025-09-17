#!/bin/bash

# Build script for matrix bindings
# This script compiles the Python bindings for the matrix controller

set -e  # Exit on any error

echo "Building Matrix Bindings"
echo "======================="

# Check if we're in the right directory
if [ ! -f "matrix_bindings.cpp" ]; then
    echo "❌ matrix_bindings.cpp not found!"
    echo "   Please run this script from the directory containing the binding files."
    exit 1
fi

# Check for required files
REQUIRED_FILES=("matrix_bindings.cpp" "matrix_wrapper.c" "matrix_wrapper.h" "librpihub75.so")
for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Required file not found: $file"
        exit 1
    fi
done

echo "✅ All required files found"

# Check Python version
PYTHON_VERSION=$(python3 --version 2>&1 | cut -d' ' -f2)
echo "✅ Python version: $PYTHON_VERSION"

# Check for pybind11
if ! python3 -c "import pybind11" 2>/dev/null; then
    echo "❌ pybind11 not found. Installing..."
    pip3 install pybind11
fi

echo "✅ pybind11 available"

# Get Python includes
echo "Getting Python includes..."
PYTHON_INCLUDES=$(python3 -m pybind11 --includes)
echo "Python includes: $PYTHON_INCLUDES"

# Get Python extension suffix
PYTHON_SUFFIX=$(python3-config --extension-suffix)
echo "Python extension suffix: $PYTHON_SUFFIX"

# Build the bindings
echo "Building Python bindings..."
g++ -O3 -Wall -shared -std=c++17 -fPIC \
    $PYTHON_INCLUDES \
    matrix_bindings.cpp matrix_wrapper.c \
    -L. -lrpihub75 \
    -o matrix_bindings$PYTHON_SUFFIX

# Check if build was successful
if [ -f "matrix_bindings$PYTHON_SUFFIX" ]; then
    echo "✅ Python bindings built successfully!"
    echo "   Output: matrix_bindings$PYTHON_SUFFIX"
else
    echo "❌ Build failed!"
    exit 1
fi

# Test the bindings
echo "Testing bindings..."
if python3 -c "import matrix_bindings; print('✅ Bindings import successful')" 2>/dev/null; then
    echo "✅ Bindings test passed!"
else
    echo "❌ Bindings test failed!"
    echo "   You may need to install the system libraries:"
    echo "   sudo cp librpihub75.so /usr/local/lib/"
    echo "   sudo cp librpihub75_gpu.so /usr/local/lib/"
    echo "   sudo ldconfig"
fi

echo ""
echo "Build completed!"
echo "You can now test the bindings with: python3 test_bindings.py"
