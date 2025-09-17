#ifndef PYTHON_BINDINGS_H
#define PYTHON_BINDINGS_H

#include "rpihub75/rpihub75.h"

// Function declarations for Python bindings
scene_info* python_init_scene(int width, int height, int fps, int brightness, 
                             int bit_depth, float gamma, const char* tone_mapper);
int python_start_render(scene_info* scene);
void python_stop_render();
void python_set_pixel(scene_info* scene, int x, int y, int r, int g, int b);
void python_clear(scene_info* scene, int r, int g, int b);
void python_cleanup(scene_info* scene);

#endif
