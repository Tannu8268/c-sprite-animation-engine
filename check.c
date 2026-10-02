/**
 * A simple test file to help you get started
 */

#include "animate.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define OUTPUT_FILE "simple.dat"

int main(void) {
    // struct canvas* canvas = animate_create_canvas(10, 10, animate_color_argb(255, 0, 0, 0));
    // struct sprite* rect = animate_create_rectangle(3, 6, animate_color_argb(255,255,255,0), 1);

    // struct sprite_placement* prect1 = animate_place_sprite(canvas, rect, 0, 0);
    // struct sprite_placement* prect2 = animate_place_sprite(canvas, rect, 2, 1);
    
    struct canvas* canvas = animate_create_canvas(20, 20, animate_color_argb(255, 0, 0, 0));
    struct sprite* rect = animate_create_rectangle(3, 6, animate_color_argb(255,255,255,0), 1);
    struct sprite* circ = animate_create_circle(4, animate_color_argb(255,255,0,0), 1);

    struct sprite_placement* prect1 = animate_place_sprite(canvas, rect, 0, 0);
    struct sprite_placement* prect2 = animate_place_sprite(canvas, rect, 2, 1);
    struct sprite_placement* pcirc  = animate_place_sprite(canvas, circ, 10, 5);
    if (prect1 == NULL || prect2 == NULL || pcirc == NULL) {
        animate_destroy_canvas(canvas);
        animate_destroy_sprite(rect);
        animate_destroy_sprite(circ);
        return EXIT_FAILURE;
    }

    size_t frame_size_bytes = animate_frame_size_bytes(canvas);
    void* data = malloc(frame_size_bytes);
    animate_generate_frame(canvas, 1, 25, data);

    FILE* fp = fopen(OUTPUT_FILE, "w");
    if (fp == NULL) {
        perror("Failed to open target file\n");
        return -1;
    }

    size_t bytes_written = fwrite(data, 1, frame_size_bytes, fp);
    if (bytes_written != frame_size_bytes) {
        printf("Failed to write buffer (%ld/%ld): %s\n", bytes_written,
               frame_size_bytes, strerror(errno));
        return -1;
    }

    fclose(fp);
    free(data);

    animate_destroy_canvas(canvas);
    animate_destroy_sprite(rect);
    animate_destroy_sprite(circ);

    return 0;
}
