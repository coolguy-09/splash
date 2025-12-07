#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <termios.h> // Added for TIOCGWINSZ

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <libsplash.h>

const float min_zoom = 0.5f; // 50%
const float max_zoom = 2.0f; // 200%

// Clamp helper
uint8_t clamp_to_byte(float v) {
	if (v < 0.0f) return 0;
	if (v > 255.0f) return 255;
	return (uint8_t)v;
}

static void clear_tty(void) {
	int clear_tty = open("/dev/tty0", O_WRONLY);
	if (clear_tty >= 0) {
		write(clear_tty, "\033[2J\033[H", 7);
		close(clear_tty);
	}
}

// Core rendering function
int splash_render(const char *image_path, const char *fb_path, int duration, int clear_flag, float scale, int debug) {
	int fb = -1;
	int tty_visible = -1;
	uint8_t *fbmem = NULL;
	size_t screensize = 0;
	int ret = 1; // assume failure until we succeed
	uint8_t *img = NULL;

	// Open framebuffer
	fb = open(fb_path, O_RDWR);
	if (fb < 0) {
		fprintf(stderr, "Libsplash: Error: Could not open %s: %s\n", fb_path, strerror(errno));
		return 1;
	}

	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;

	if (ioctl(fb, FBIOGET_FSCREENINFO, &finfo) < 0) {
		fprintf(stderr, "Libsplash: Error: FBIOGET_FSCREENINFO failed: %s\n", strerror(errno));
		goto cleanup;
	}
	if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0) {
		fprintf(stderr, "Libsplash: Error: FBIOGET_VSCREENINFO failed: %s\n", strerror(errno));
		goto cleanup;
	}

	// Compute scaled placement
	if (scale > max_zoom) {
		fprintf(stderr, "Libsplash: Error: Zoom value too high\n");
		goto cleanup;
	} else if (scale < min_zoom) {
		fprintf(stderr, "Libsplash: Error: Zoom value too low\n");
		goto cleanup;
	}

	// Hide cursor (do this early, and always unhide on exit)
	{
		int tty = open("/dev/tty0", O_WRONLY);
		if (tty >= 0) {
			write(tty, "\033[?25l", 6);
			close(tty);
			tty_visible = 0; // we hid it
		}
	}

	if (debug) {
		printf("Libsplash: Display: %dx%d, %d bpp, stride: %d bytes\n",
			   vinfo.xres, vinfo.yres, vinfo.bits_per_pixel, finfo.line_length);
	}

	// Map framebuffer
	screensize = finfo.smem_len;
	fbmem = mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
	if (fbmem == MAP_FAILED) {
		fprintf(stderr, "Libsplash: Error: Couldn't mmap framebuffer: %s\n", strerror(errno));
		fbmem = NULL;
		goto cleanup;
	}

	if (clear_flag) {
		clear_tty();
		// FIX 1: Wait 50ms for TTY to finish clearing so we don't draw on a busy screen
		usleep(50000); 

		if (debug) {
			printf("Libsplash: Clearing framebuffer memory...\n");
		}
		memset(fbmem, 0, screensize);
	}

	// Load image
	int img_w = 0, img_h = 0, channels = 0;
	img = stbi_load(image_path, &img_w, &img_h, &channels, 3);
	if (!img) {
		fprintf(stderr, "Libsplash: Error: Couldn't load image %s\n", image_path);
		goto cleanup;
	}

	const float s = scale > 0.0f ? scale : 1.0f;
	const int scaled_w = (int)(img_w * s);
	const int scaled_h = (int)(img_h * s);
	const int x0 = (int)((int)vinfo.xres - scaled_w) / 2;
	const int y0 = (int)((int)vinfo.yres - scaled_h) / 2;

	// Render
	for (int y = 0; y < scaled_h; y++) {
		for (int x = 0; x < scaled_w; x++) {
			float f_src_x = (float)x / s;
			float f_src_y = (float)y / s;

			int x1 = (int)f_src_x;
			int y1 = (int)f_src_y;
			float dx = f_src_x - x1;
			float dy = f_src_y - y1;

			int x2 = (x1 + 1 < img_w) ? x1 + 1 : x1;
			int y2 = (y1 + 1 < img_h) ? y1 + 1 : y1;

			int idx1 = (y1 * img_w + x1) * 3;
			int idx2 = (y1 * img_w + x2) * 3;
			int idx3 = (y2 * img_w + x1) * 3;
			int idx4 = (y2 * img_w + x2) * 3;

			float R1 = img[idx1 + 0] * (1.0f - dx) + img[idx2 + 0] * dx;
			float R2 = img[idx3 + 0] * (1.0f - dx) + img[idx4 + 0] * dx;
			float G1 = img[idx1 + 1] * (1.0f - dx) + img[idx2 + 1] * dx;
			float G2 = img[idx3 + 1] * (1.0f - dx) + img[idx4 + 1] * dx;
			float B1 = img[idx1 + 2] * (1.0f - dx) + img[idx2 + 2] * dx;
			float B2 = img[idx3 + 2] * (1.0f - dx) + img[idx4 + 2] * dx;

			uint8_t r = clamp_to_byte(R1 * (1.0f - dy) + R2 * dy);
			uint8_t g = clamp_to_byte(G1 * (1.0f - dy) + G2 * dy);
			uint8_t b = clamp_to_byte(B1 * (1.0f - dy) + B2 * dy);

			int sx = x0 + x;
			int sy = y0 + y;
			if (sx < 0 || sx >= (int)vinfo.xres || sy < 0 || sy >= (int)vinfo.yres) {
				continue;
			}

			long location = (sy + vinfo.yoffset) * finfo.line_length +
							(sx + vinfo.xoffset) * (vinfo.bits_per_pixel / 8);

			if (vinfo.bits_per_pixel == 32 || vinfo.bits_per_pixel == 24) {
				int r_off = vinfo.red.offset   / 8;
				int g_off = vinfo.green.offset / 8;
				int b_off = vinfo.blue.offset  / 8;

				// Write only the color channels per offsets
				fbmem[location + r_off] = r;
				fbmem[location + g_off] = g;
				fbmem[location + b_off] = b;

				// If transparency channel exists, set it to opaque
				if (vinfo.transp.length > 0) {
					int a_off = vinfo.transp.offset / 8;
					fbmem[location + a_off] = 0xFF;
				}
			} else if (vinfo.bits_per_pixel == 16) {
				if (vinfo.green.length == 6) {
					uint16_t rgb565 = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
					*((uint16_t *)(fbmem + location)) = rgb565;
				} else if (vinfo.green.length == 5) {
					uint16_t rgb555 = (uint16_t)(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
					*((uint16_t *)(fbmem + location)) = rgb555;
				}
			}
		}
	}

	// Keep image on screen or handle immediate exit
	if (duration > 0) {
		sleep(duration);
	}
	int tty = open("/dev/tty0", O_RDWR);
	if (tty >= 0) {
		struct winsize w;
		if (ioctl(tty, TIOCGWINSZ, &w) >= 0) {
			// Move cursor to the last row
			dprintf(tty, "\033[%d;1H", w.ws_row);
		}
		close(tty);
	}

	ret = 0; // success

cleanup:

	// Unmap and close
	if (img) stbi_image_free(img);
	if (fbmem) munmap(fbmem, screensize);
	if (fb >= 0) close(fb);

	// If we hid the cursor but couldn’t restore above (error path), try now
	if (tty_visible == 0) {
		printf("\033[?25h");
		fflush(stdout);
	}

	return ret;
}
