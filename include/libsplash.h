#ifndef LIBSPLASH_H
#define LIBSPLASH_H

#include <stdint.h>
#include <stddef.h>

extern const float min_zoom;
extern const float max_zoom;
extern int max_time;

// Clamp helper
uint8_t clamp_to_byte(float v);

// Core rendering function
int splash_render(const char *image_path, const char *fb_path, int duration, int clear_flag, float scale, int debug);

#endif

