#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>

// Include our wrapper header
extern "C" {
#include "matrix_wrapper.h"
}

namespace py = pybind11;

class MatrixController {
private:
    matrix_wrapper_t* wrapper;
    std::thread render_thread;
    std::atomic<bool> running;
    
public:
    MatrixController(int width, int height, int brightness = 50, int fps = 60, 
                    int bit_depth = 8, double gamma = 2.2, const std::string& tone_mapper = "aces",
                    int panel_width = -1, int panel_height = -1, const std::string& pixel_order = "RGB",
                    int num_ports = 1, int num_chains = 1, int dither_level = 0,
                    int motion_blur_frames = 0, const std::string& image_mapper = "u",
                    const std::string& shader_file = "") 
        : wrapper(nullptr), running(false) {
        
        // Create the C wrapper with all parameters
        int actual_panel_width = (panel_width == -1) ? width : panel_width;
        int actual_panel_height = (panel_height == -1) ? height : panel_height;
        
        wrapper = matrix_wrapper_create(width, height, brightness, fps, bit_depth, gamma, tone_mapper.c_str(),
                                      actual_panel_width, actual_panel_height, pixel_order.c_str(), num_ports, num_chains,
                                      dither_level, motion_blur_frames, image_mapper.c_str(), 
                                      shader_file.empty() ? nullptr : shader_file.c_str());
        if (!wrapper) {
            throw std::runtime_error("Failed to initialize matrix wrapper");
        }
    }
    
    ~MatrixController() {
        stop();
        if (wrapper) {
            matrix_wrapper_destroy(wrapper);
        }
    }
    
        void start() {
            if (running) return;

            if (!matrix_wrapper_start(wrapper)) {
                throw std::runtime_error("Failed to start matrix wrapper");
            }

            running = true;
            render_thread = std::thread([this]() {
                // Simple update loop that calls bcm_mapper repeatedly
                while (running) {
                    matrix_wrapper_update(wrapper);
                    std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
                }
            });
        }
    
    void stop() {
        if (!running) return;
        
        running = false;
        if (render_thread.joinable()) {
            render_thread.join();
        }
        matrix_wrapper_stop(wrapper);
    }
    
    void set_pixel(int x, int y, int r, int g, int b) {
        matrix_wrapper_set_pixel(wrapper, x, y, r, g, b);
    }
    
    void clear() {
        matrix_wrapper_clear(wrapper);
    }
    
    void set_pixels(py::array_t<uint8_t> pixels) {
        auto buf = pixels.request();
        if (buf.ndim != 3 || buf.shape[2] != 3) {
            throw std::runtime_error("Expected 3D array with shape (height, width, 3)");
        }
        
        int height = buf.shape[0];
        int width = buf.shape[1];
        
        if (width != get_width() || height != get_height()) {
            throw std::runtime_error("Pixel array dimensions don't match scene dimensions");
        }
        
        // Copy pixels directly to the wrapper's buffer
        uint8_t* ptr = static_cast<uint8_t*>(buf.ptr);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int src_index = (y * width + x) * 3;
                set_pixel(x, y, ptr[src_index], ptr[src_index + 1], ptr[src_index + 2]);
            }
        }
    }
    
    int get_width() const { return matrix_wrapper_get_width(wrapper); }
    int get_height() const { return matrix_wrapper_get_height(wrapper); }
    bool is_running() const { return running && matrix_wrapper_is_running(wrapper); }
};

PYBIND11_MODULE(matrix_bindings, m) {
    m.doc() = "Python bindings for rpi-gpu-hub75-matrix library";
    
    py::class_<MatrixController>(m, "MatrixController")
        .def(py::init<int, int, int, int, int, double, std::string, int, int, std::string, int, int, int, int, std::string, std::string>(),
             py::arg("width"), py::arg("height"), 
             py::arg("brightness") = 50, py::arg("fps") = 60,
             py::arg("bit_depth") = 8, py::arg("gamma") = 2.2, 
             py::arg("tone_mapper") = "aces",
             py::arg("panel_width") = -1, py::arg("panel_height") = -1,
             py::arg("pixel_order") = "RGB", py::arg("num_ports") = 1,
             py::arg("num_chains") = 1, py::arg("dither_level") = 0,
             py::arg("motion_blur_frames") = 0, py::arg("image_mapper") = "u",
             py::arg("shader_file") = "")
        .def("start", &MatrixController::start)
        .def("stop", &MatrixController::stop)
        .def("set_pixel", &MatrixController::set_pixel)
        .def("clear", &MatrixController::clear)
        .def("set_pixels", &MatrixController::set_pixels)
        .def("get_width", &MatrixController::get_width)
        .def("get_height", &MatrixController::get_height)
        .def("is_running", &MatrixController::is_running);
}
