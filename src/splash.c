#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsplash.h>

int verbose = 0;

void print_usage(void) {
	printf("Help for splash:\n");
	printf("  splash [OPTIONS] <image>\n");
	printf("  Options:\n");
	printf("    -v, --verbose: Enable verbose messages\n");
	printf("    -c, --clear: Clear screen before printing\n");
	printf("    -d <seconds>, --duration <seconds>: Wait for <seconds> before exiting\n");
	printf("    -f <device>, --framebuffer <device>: Specify framebuffer device\n");
	printf("    -z <percent>, --zoom <percent>: Specify zoom for image (50-200%)\n");
	printf("    -h, --help: Show this help dialouge\n");
	printf("    -V, --version: Print the version of splash\n");
}

void print_version(void) {
	printf("splash v1.0\n");
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Invalid usage, see --help for help.\n");
		return 1;
	}

	float scale_factor = 1.0f; // Default to 100% (1x) zoom
	const char *image_path = NULL;
	int duration = 0;
	int clear_flag = 0;
	const char *framebuffer = "/dev/fb0"; // Default to /dev/fb0 if -f isnt used

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_usage();
			return 0;
		} else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
			print_version();
			return 0;
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
			verbose = 1;
		} else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--clear") == 0) {
			clear_flag = 1;
		} else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--framebuffer") == 0) {
			if (i + 1 < argc) {
				framebuffer = argv[++i];
			} else {
				fprintf(stderr, "Error: %s option requires a value (path)\n", argv[i]);
				fprintf(stderr, "Please see --help for help.\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--zoom") == 0) {
			if (i + 1 < argc) {
				scale_factor = atof(argv[++i]) / 100.0f;
			} else {
				fprintf(stderr, "Error: %s option requires a value (percent)\n", argv[i]);
				fprintf(stderr, "Please see --help for help.\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--duration") == 0) {
			if (i + 1 < argc) {
				duration = atoi(argv[++i]);
			} else {
				fprintf(stderr, "Error: %s option requires a value (seconds)\n", argv[i]);
				fprintf(stderr, "Please see --help for help.\n");
				return 1;
			}
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "Error: unrecognized option '%s'\n", argv[i]);
			fprintf(stderr, "Please see --help for help.\n");
			return 1;
		} else {
			if (image_path == NULL) {
				image_path = argv[i];
			} else {
				fprintf(stderr, "Error: unexpected argument: '%s'\n", argv[i]);
				fprintf(stderr, "Please see --help for help.\n");
				return 1;
			}
		}
	}

	if (scale_factor > max_zoom) {
		fprintf(stderr, "Error: Zoom value too high\n");
		return 1;
	} else if (scale_factor < min_zoom) {
		fprintf(stderr, "Error: Zoom value too low\n");
		return 1;
	}

	if (image_path == NULL) {
		fprintf(stderr, "Invalid usage, see --help for help.\n");
		return 1;
	}
	
	// Call into the library
	return splash_render(image_path, framebuffer, duration, clear_flag, scale_factor, verbose);
}

