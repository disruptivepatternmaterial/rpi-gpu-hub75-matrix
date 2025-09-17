#!/usr/bin/env python3
"""
Python Matrix Controller using pybind11 bindings
Direct Python bindings to the C library for optimal performance
"""

import numpy as np
import time
from typing import Optional

# Import the compiled pybind11 module
try:
    import matrix_bindings
except ImportError:
    print("Error: matrix_bindings module not found. Please compile it first:")
    print("python3 setup_pybind11.py build_ext --inplace")
    exit(1)

class RGB:
    """RGB color class"""
    def __init__(self, r: int, g: int, b: int):
        self.r = max(0, min(255, r))
        self.g = max(0, min(255, g))
        self.b = max(0, min(255, b))

class PythonMatrixControllerPybind11:
    """Matrix controller using pybind11 bindings for direct C library access."""
    
    def __init__(self, 
                 # Basic dimensions (required)
                 width: int, 
                 height: int,
                 
                 # Basic settings (required)
                 brightness: int = 50,
                 fps: int = 60,
                 bit_depth: int = 8,
                 gamma: float = 2.2,
                 tone_mapper: str = "aces",
                 
                 # Panel configuration (optional)
                 panel_width: int = None,
                 panel_height: int = None,
                 pixel_order: str = "RGB",
                 
                 # Performance settings (optional)
                 num_ports: int = 1,
                 num_chains: int = 1,
                 
                 # Display quality settings (optional)
                 dither_level: int = 0,
                 motion_blur_frames: int = 0,
                 
                 # Image processing (optional)
                 image_mapper: str = "u",
                 
                 # Shader/video source (optional)
                 shader_file: str = None):
        
        # Store all configuration parameters
        self.width = width
        self.height = height
        self.panel_width = panel_width if panel_width is not None else width
        self.panel_height = panel_height if panel_height is not None else height
        self.pixel_order = pixel_order
        self.fps = fps
        self.num_ports = num_ports
        self.num_chains = num_chains
        self.brightness = brightness
        self.gamma = gamma
        self.bit_depth = bit_depth
        self.dither_level = dither_level
        self.motion_blur_frames = motion_blur_frames
        self.image_mapper = image_mapper
        self.tone_mapper = tone_mapper
        self.shader_file = shader_file
        
        # Validate parameters
        self._validate_parameters()
        
        # Create the C++ matrix controller with all parameters
        self.controller = matrix_bindings.MatrixController(
            width, height, brightness, fps, bit_depth, gamma, tone_mapper,
            self.panel_width, self.panel_height, pixel_order, num_ports, num_chains,
            dither_level, motion_blur_frames, image_mapper, 
            shader_file if shader_file else ""
        )
        
        print(f"✅ Pybind11 Matrix Controller initialized:")
        print(f"   📐 Matrix: {self.width}x{self.height}")
        print(f"   📱 Panel: {self.panel_width}x{self.panel_height}")
        print(f"   🎨 Pixel Order: {self.pixel_order}")
        print(f"   ⚡ FPS: {self.fps}")
        print(f"   🔌 Ports: {self.num_ports}, Chains: {self.num_chains}")
        print(f"   💡 Brightness: {self.brightness}%, Gamma: {self.gamma}")
        print(f"   🎯 Bit Depth: {self.bit_depth}, Dither: {self.dither_level}")
        print(f"   🎬 Motion Blur: {self.motion_blur_frames} frames")
        print(f"   🖼️  Image Mapper: {self.image_mapper}")
        print(f"   🎨 Tone Mapper: {self.tone_mapper}")
        if self.shader_file:
            print(f"   🎮 Shader: {self.shader_file}")
        print(f"🚀 Using direct C library bindings for maximum performance")
    
    def _validate_parameters(self):
        """Validate configuration parameters."""
        # Validate dimensions
        if not (16 <= self.width <= 512):
            raise ValueError(f"Width must be between 16-512, got {self.width}")
        if not (16 <= self.height <= 512):
            raise ValueError(f"Height must be between 16-512, got {self.height}")
        
        # Validate panel dimensions
        valid_panel_widths = [16, 32, 64, 128]
        if self.panel_width not in valid_panel_widths:
            raise ValueError(f"Panel width must be one of {valid_panel_widths}, got {self.panel_width}")
        
        valid_panel_heights = [16, 32, 64]
        if self.panel_height not in valid_panel_heights:
            raise ValueError(f"Panel height must be one of {valid_panel_heights}, got {self.panel_height}")
        
        # Validate pixel order
        valid_pixel_orders = ["RGB", "RBG", "BGR"]
        if self.pixel_order not in valid_pixel_orders:
            raise ValueError(f"Pixel order must be one of {valid_pixel_orders}, got {self.pixel_order}")
        
        # Validate performance settings
        if not (1 <= self.fps <= 255):
            raise ValueError(f"FPS must be between 1-255, got {self.fps}")
        if not (1 <= self.num_ports <= 3):
            raise ValueError(f"Number of ports must be between 1-3, got {self.num_ports}")
        if not (1 <= self.num_chains <= 16):
            raise ValueError(f"Number of chains must be between 1-16, got {self.num_chains}")
        
        # Validate quality settings
        if not (0 <= self.brightness <= 254):
            raise ValueError(f"Brightness must be between 0-254, got {self.brightness}")
        if not (1.0 <= self.gamma <= 2.8):
            raise ValueError(f"Gamma must be between 1.0-2.8, got {self.gamma}")
        if not (2 <= self.bit_depth <= 64):
            raise ValueError(f"Bit depth must be between 2-64, got {self.bit_depth}")
        if not (0 <= self.dither_level <= 10):
            raise ValueError(f"Dither level must be between 0-10, got {self.dither_level}")
        if not (0 <= self.motion_blur_frames <= 32):
            raise ValueError(f"Motion blur frames must be between 0-32, got {self.motion_blur_frames}")
        
        # Validate image mapper
        valid_image_mappers = ["mirror", "flip", "mirror_flip", "u"]
        if self.image_mapper not in valid_image_mappers:
            raise ValueError(f"Image mapper must be one of {valid_image_mappers}, got {self.image_mapper}")
        
        # Validate tone mapper
        valid_tone_mappers = ["aces", "reinhard", "none", "saturation", "sigmoid", "hable"]
        if self.tone_mapper not in valid_tone_mappers:
            raise ValueError(f"Tone mapper must be one of {valid_tone_mappers}, got {self.tone_mapper}")
    
    def update_config(self, **kwargs):
        """Update configuration parameters after initialization."""
        for key, value in kwargs.items():
            if hasattr(self, key):
                setattr(self, key, value)
            else:
                raise ValueError(f"Unknown parameter: {key}")
        
        # Re-validate and recreate controller if needed
        self._validate_parameters()
        # Note: In a full implementation, you'd recreate the controller here
        print(f"✅ Configuration updated: {kwargs}")
    
    def start(self):
        """Start the matrix display system."""
        try:
            self.controller.start()
            print("✅ Pybind11 Matrix started successfully!")
            return True
        except Exception as e:
            print(f"Failed to start Pybind11 matrix: {e}")
            return False
    
    def stop(self):
        """Stop the matrix display system."""
        try:
            self.controller.stop()
            print("✅ Pybind11 Matrix stopped")
        except Exception as e:
            print(f"Error stopping Pybind11 matrix: {e}")
    
    def clear(self):
        """Clear the matrix (set all pixels to black)."""
        self.controller.clear()
    
    def set_pixel(self, x: int, y: int, r: int, g: int, b: int):
        """Set a pixel at position (x, y) to color (r, g, b)."""
        self.controller.set_pixel(x, y, r, g, b)
    
    def set_pixels(self, pixels: np.ndarray):
        """Set multiple pixels from a numpy array.
        
        Args:
            pixels: numpy array with shape (height, width, 3) containing RGB values
        """
        if pixels.shape != (self.height, self.width, 3):
            raise ValueError(f"Expected array shape ({self.height}, {self.width}, 3), got {pixels.shape}")
        
        self.controller.set_pixels(pixels)
    
    def get_width(self) -> int:
        """Get the matrix width."""
        return self.controller.get_width()
    
    def get_height(self) -> int:
        """Get the matrix height."""
        return self.controller.get_height()
    
    def is_running(self) -> bool:
        """Check if the matrix is running."""
        return self.controller.is_running()
    
    def __del__(self):
        """Cleanup when object is destroyed."""
        self.stop()

# Test function
def test_pybind11_controller():
    """Test the pybind11 controller."""
    print("Testing Pybind11 Matrix Controller...")
    
    controller = PythonMatrixControllerPybind11()
    
    try:
        if not controller.start():
            print("Failed to start controller")
            return
        
        print("Drawing test pattern...")
        
        # Draw a red square
        for y in range(10, 22):
            for x in range(10, 22):
                controller.set_pixel(x, y, 255, 0, 0)
        
        # Draw a green square
        for y in range(10, 22):
            for x in range(30, 42):
                controller.set_pixel(x, y, 0, 255, 0)
        
        # Draw a blue square
        for y in range(10, 22):
            for x in range(50, 62):
                controller.set_pixel(x, y, 0, 0, 255)
        
        print("Test pattern drawn. Check the matrix!")
        print("Press Ctrl+C to stop...")
        
        # Keep running
        while True:
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\nStopping test...")
    finally:
        controller.stop()

if __name__ == "__main__":
    test_pybind11_controller()
