#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>

// Include the original headers
#include "../rpi-gpu-hub75-matrix/include/rpihub75.h"
#include "../rpi-gpu-hub75-matrix/include/util.h"
#include "../rpi-gpu-hub75-matrix/include/pixels.h"

// Simple wrapper structure for Python
typedef struct {
    scene_info* scene;
    uint8_t* pixel_buffer;
    int width;
    int height;
    bool running;
    pthread_t render_thread;
} matrix_wrapper_t;

// Create a new matrix wrapper
matrix_wrapper_t* matrix_wrapper_create(int width, int height, int brightness, 
                                       int fps, int bit_depth, double gamma, 
                                       const char* tone_mapper,
                                       int panel_width, int panel_height, 
                                       const char* pixel_order, int num_ports, 
                                       int num_chains, int dither_level, 
                                       int motion_blur_frames, const char* image_mapper,
                                       const char* shader_file) {
    matrix_wrapper_t* wrapper = malloc(sizeof(matrix_wrapper_t));
    if (!wrapper) return NULL;
    
    // Initialize pixel buffer
    wrapper->pixel_buffer = malloc(width * height * 3);
    if (!wrapper->pixel_buffer) {
        free(wrapper);
        return NULL;
    }
    
    wrapper->width = width;
    wrapper->height = height;
    wrapper->running = false;
    wrapper->render_thread = 0;
    
    // Create command line arguments - start with basic working set
    char* args[] = {
        "matrix_controller",
        "-x", malloc(32), "-y", malloc(32),
        "-w", malloc(32), "-h", malloc(32),
        "-O", (char*)pixel_order,
        "-f", malloc(32),
        "-p", malloc(32), "-c", malloc(32),
        "-d", malloc(32), "-g", malloc(32),
        "-b", malloc(32), "-l", malloc(32),
        "-m", malloc(32), "-i", (char*)image_mapper,
        "-t", malloc(32),
        NULL
    };
    
    // Fill in the arguments
    snprintf(args[2], 32, "%d", width);           // -x total width
    snprintf(args[4], 32, "%d", height);          // -y total height
    snprintf(args[6], 32, "%d", panel_width);     // -w panel width
    snprintf(args[8], 32, "%d", panel_height);    // -h panel height
    snprintf(args[12], 32, "%d", fps);            // -f fps
    snprintf(args[14], 32, "%d", num_ports);      // -p num ports
    snprintf(args[16], 32, "%d", num_chains);     // -c num chains
    snprintf(args[18], 32, "%d", bit_depth);      // -d bit depth
    snprintf(args[20], 32, "%.2f", gamma);        // -g gamma
    snprintf(args[22], 32, "%d", brightness);     // -b brightness
    snprintf(args[24], 32, "%d", dither_level);   // -l dither level
    snprintf(args[26], 32, "%d", motion_blur_frames); // -m motion blur
    snprintf(args[30], 32, "%s", tone_mapper);        // -t tone mapper
    
    // Initialize scene
    int arg_count = 31;
    wrapper->scene = default_scene(arg_count, args);
    if (!wrapper->scene) {
        free(wrapper->pixel_buffer);
        free(wrapper);
        return NULL;
    }
    
    // Allocate image buffer for the scene if not already allocated
    if (!wrapper->scene->image) {
        wrapper->scene->image = malloc(wrapper->scene->width * wrapper->scene->height * wrapper->scene->stride);
        if (!wrapper->scene->image) {
            free(wrapper->pixel_buffer);
            free(wrapper->scene);
            free(wrapper);
            return NULL;
        }
    }
    
    // Free allocated argument strings
    for (int i = 0; i < arg_count; i++) {
        if (i == 2 || i == 4 || i == 6 || i == 8 || i == 12 || i == 14 || 
            i == 16 || i == 18 || i == 20 || i == 22 || i == 24 || i == 26 || i == 30) {
            if (args[i]) free(args[i]);
        }
    }
    
    return wrapper;
}

// Destroy matrix wrapper
void matrix_wrapper_destroy(matrix_wrapper_t* wrapper) {
    if (!wrapper) return;
    
    if (wrapper->scene) {
        free(wrapper->scene);
    }
    
    if (wrapper->pixel_buffer) {
        free(wrapper->pixel_buffer);
    }
    
    free(wrapper);
}

// Render thread function that runs render_forever
void* render_thread_func(void* arg) {
    matrix_wrapper_t* wrapper = (matrix_wrapper_t*)arg;
    if (!wrapper || !wrapper->scene) return NULL;
    
    // Wait a moment for initialization
    usleep(50000);
    
    // Run the main rendering loop that drives the hardware
    render_forever(wrapper->scene);
    return NULL;
}

// Start the matrix
bool matrix_wrapper_start(matrix_wrapper_t* wrapper) {
    if (!wrapper || wrapper->running) return false;
    
    // Check that the scene is properly initialized
    if (!wrapper->scene) {
        return false;
    }
    
    wrapper->running = true;
    
    // Start the render thread that will drive the hardware
    if (pthread_create(&wrapper->render_thread, NULL, render_thread_func, wrapper) != 0) {
        wrapper->running = false;
        return false;
    }
    
    return true;
}

// Stop the matrix
void matrix_wrapper_stop(matrix_wrapper_t* wrapper) {
    if (wrapper) {
        wrapper->running = false;
        
        // Wait for render thread to finish
        if (wrapper->render_thread != 0) {
            pthread_join(wrapper->render_thread, NULL);
            wrapper->render_thread = 0;
        }
    }
}

// Set a pixel
void matrix_wrapper_set_pixel(matrix_wrapper_t* wrapper, int x, int y, int r, int g, int b) {
    if (!wrapper || !wrapper->pixel_buffer) return;
    
    if (x >= 0 && x < wrapper->width && y >= 0 && y < wrapper->height) {
        int index = (y * wrapper->width + x) * 3;
        wrapper->pixel_buffer[index] = r;
        wrapper->pixel_buffer[index + 1] = g;
        wrapper->pixel_buffer[index + 2] = b;
    }
}

// Clear the matrix
void matrix_wrapper_clear(matrix_wrapper_t* wrapper) {
    if (!wrapper || !wrapper->pixel_buffer) return;
    
    memset(wrapper->pixel_buffer, 0, wrapper->width * wrapper->height * 3);
    
    // Also clear the scene image buffer if it exists
    if (wrapper->scene && wrapper->scene->image) {
        memset(wrapper->scene->image, 0, wrapper->width * wrapper->height * 3);
    }
}

// Update the display
void matrix_wrapper_update(matrix_wrapper_t* wrapper) {
    if (!wrapper || !wrapper->running || !wrapper->scene || !wrapper->pixel_buffer) return;
    
    // Copy our pixel buffer to the scene's image buffer
    if (wrapper->scene->image) {
        memcpy(wrapper->scene->image, wrapper->pixel_buffer, 
               wrapper->width * wrapper->height * 3);
    }
    
    // Update the display
    wrapper->scene->bcm_mapper(wrapper->scene, NULL);
    calculate_fps(wrapper->scene->fps, false);
}

// Run the main rendering loop (this should be called once, not in a loop)
void matrix_wrapper_render_forever(matrix_wrapper_t* wrapper) {
    if (!wrapper || !wrapper->scene) return;
    
    // This is the main rendering loop that handles GPIO and matrix output
    render_forever(wrapper->scene);
}

// Get dimensions
int matrix_wrapper_get_width(matrix_wrapper_t* wrapper) {
    return wrapper ? wrapper->width : 0;
}

int matrix_wrapper_get_height(matrix_wrapper_t* wrapper) {
    return wrapper ? wrapper->height : 0;
}

bool matrix_wrapper_is_running(matrix_wrapper_t* wrapper) {
    return wrapper ? wrapper->running : false;
}
