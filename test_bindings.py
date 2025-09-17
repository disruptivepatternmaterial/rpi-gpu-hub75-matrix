#!/usr/bin/env python3
"""
Simple test script for the matrix bindings.
This script tests the basic functionality of the Python bindings.
"""

import time
import sys

def test_bindings():
    """Test the matrix bindings functionality."""
    print("Matrix Bindings Test")
    print("=" * 25)
    
    try:
        # Import the bindings
        from python_matrix_controller_pybind11 import PythonMatrixControllerPybind11
        print("✅ Bindings imported successfully")
        
        # Create matrix controller
        print("Creating matrix controller...")
        matrix = PythonMatrixControllerPybind11(
            width=64, 
            height=32, 
            brightness=50,
            fps=60
        )
        print("✅ Matrix controller created")
        
        # Start the matrix
        print("Starting matrix...")
        matrix.start()
        print("✅ Matrix started")
        
        # Test basic pixel operations
        print("Testing pixel operations...")
        
        # Clear the matrix
        matrix.clear()
        print("✅ Matrix cleared")
        
        # Draw a red pixel
        matrix.set_pixel(0, 0, 255, 0, 0)
        print("✅ Red pixel set")
        
        # Draw a green pixel
        matrix.set_pixel(1, 0, 0, 255, 0)
        print("✅ Green pixel set")
        
        # Draw a blue pixel
        matrix.set_pixel(2, 0, 0, 0, 255)
        print("✅ Blue pixel set")
        
        # Update display
        matrix.swap_on_vsync()
        print("✅ Display updated")
        
        # Wait a moment
        print("Waiting 3 seconds...")
        time.sleep(3)
        
        # Test drawing a pattern
        print("Drawing test pattern...")
        matrix.clear()
        
        # Draw a simple pattern
        for y in range(32):
            for x in range(64):
                if (x + y) % 4 == 0:
                    matrix.set_pixel(x, y, 255, 255, 255)
        
        matrix.swap_on_vsync()
        print("✅ Test pattern drawn")
        
        # Wait a moment
        print("Waiting 3 seconds...")
        time.sleep(3)
        
        # Test color cycling
        print("Testing color cycling...")
        colors = [
            (255, 0, 0),    # Red
            (0, 255, 0),    # Green
            (0, 0, 255),    # Blue
            (255, 255, 0),  # Yellow
            (255, 0, 255),  # Magenta
            (0, 255, 255),  # Cyan
            (255, 255, 255) # White
        ]
        
        for color in colors:
            matrix.clear()
            # Fill with current color
            for y in range(32):
                for x in range(64):
                    matrix.set_pixel(x, y, *color)
            matrix.swap_on_vsync()
            time.sleep(0.5)
        
        print("✅ Color cycling completed")
        
        # Stop the matrix
        print("Stopping matrix...")
        matrix.stop()
        print("✅ Matrix stopped")
        
        print("\n🎉 All tests passed! The bindings are working correctly.")
        return True
        
    except ImportError as e:
        print(f"❌ Failed to import bindings: {e}")
        print("Make sure the bindings are compiled and installed correctly.")
        return False
        
    except Exception as e:
        print(f"❌ Test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Main test function."""
    print("Testing Matrix Bindings")
    print("=" * 30)
    
    # Check if running on Raspberry Pi
    try:
        with open('/proc/cpuinfo', 'r') as f:
            if 'Raspberry Pi' not in f.read():
                print("⚠️  Warning: This test is designed for Raspberry Pi")
                print("   The hardware-specific parts may not work on other systems.")
    except:
        print("⚠️  Warning: Could not detect system type")
    
    # Run the test
    success = test_bindings()
    
    if success:
        print("\n✅ Bindings test completed successfully!")
        sys.exit(0)
    else:
        print("\n❌ Bindings test failed!")
        sys.exit(1)

if __name__ == "__main__":
    main()
