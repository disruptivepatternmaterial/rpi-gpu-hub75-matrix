#ifndef MATRIX_WRAPPER_H
#define MATRIX_WRAPPER_H

#include <stdbool.h>

// Forward declaration
typedef struct matrix_wrapper_t matrix_wrapper_t;

// Function declarations
matrix_wrapper_t* matrix_wrapper_create(int width, int height, int brightness, 
                                       int fps, int bit_depth, double gamma, 
                                       const char* tone_mapper,
                                       int panel_width, int panel_height, 
                                       const char* pixel_order, int num_ports, 
                                       int num_chains, int dither_level, 
                                       int motion_blur_frames, const char* image_mapper,
                                       const char* shader_file);
void matrix_wrapper_destroy(matrix_wrapper_t* wrapper);
bool matrix_wrapper_start(matrix_wrapper_t* wrapper);
void matrix_wrapper_stop(matrix_wrapper_t* wrapper);
void matrix_wrapper_set_pixel(matrix_wrapper_t* wrapper, int x, int y, int r, int g, int b);
void matrix_wrapper_clear(matrix_wrapper_t* wrapper);
void matrix_wrapper_update(matrix_wrapper_t* wrapper);
void matrix_wrapper_render_forever(matrix_wrapper_t* wrapper);
int matrix_wrapper_get_width(matrix_wrapper_t* wrapper);
int matrix_wrapper_get_height(matrix_wrapper_t* wrapper);
bool matrix_wrapper_is_running(matrix_wrapper_t* wrapper);

#endif // MATRIX_WRAPPER_H