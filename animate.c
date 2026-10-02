#include "animate.h"
#include <stdlib.h>  
#include <stdio.h>


struct sprite {
    size_t width, height;
    color_t* pixels;  
    int ref_count; //to check the num of placements using this sprite  
};

struct sprite_placement {
    struct sprite* sprite;
    ssize_t x, y; //initial position
    ssize_t vx, vy; //velocities
    ssize_t ax, ay; //acceleration 
    struct canvas* canvas; 
    struct sprite_placement* previous;
    struct sprite_placement* next;   
};

struct canvas {
    size_t height, width; 
    color_t background; 
    struct sprite_placement* top; //top placement in the list 
    struct sprite_placement* bottom;  //bottom placement in the list 
};


/* This is the header structure that we can expect to find at position 0 in a bitmap file. */
struct bitmap_header {
    uint8_t  magic[2];          // Expect {'B', 'M'}
    uint32_t size_bytes;        // Size of the file in bytes
    uint16_t reserved[2];
    uint32_t pixel_offset;      // Starting address of pixel data
// Don't pad this struct for alignment
} __attribute__((packed));

/*
 * This header immediately follows bitmap_header in the file.
 * You may ignore all fields except bV5Width and bV5Height, unless you'd like to validate
 * the image format
 */
struct bitmapv5_header {
                               // Offset     Size Description
    uint32_t bV5Size         ; // 0x00          4 Size of this header (124 bytes)
    uint32_t bV5Width        ; // 0x04          4 Width of the bitmap in pixels
    uint32_t bV5Height       ; // 0x08          4 Height of the bitmap in pixels
    uint16_t bV5Planes       ; // 0x0C          2 Number of planes (must be 1)
    uint16_t bV5BitCount     ; // 0x0E          2 Bits per pixel (e.g., 32)
    uint32_t bV5Compression  ; // 0x10          4 BI_RGB (0), BI_BITFIELDS (3)
    uint32_t bV5SizeImage    ; // 0x14          4 Size of image data (0 if uncompressed)
    uint32_t bV5XPelsPerMeter; // 0x18          4 Horizontal pixels per meter
    uint32_t bV5YPelsPerMeter; // 0x1C          4 Vertical pixels per meter
    uint32_t bV5ClrUsed      ; // 0x20          4 Number of color indices used
    uint32_t bV5ClrImportant ; // 0x24          4 Number of important colors
    uint32_t bV5RedMask      ; // 0x28          4 Color mask for red component
    uint32_t bV5GreenMask    ; // 0x2C          4 Color mask for green component
    uint32_t bV5BlueMask     ; // 0x30          4 Color mask for blue component
    uint32_t bV5AlphaMask    ; // 0x34          4 Color mask for alpha channel
    uint32_t bV5CSType       ; // 0x38          4 Color space type (e.g., LCS_CALIBRATED_RGB)
    uint8_t  bV5Endpoints[36]; // 0x3C-0x5B    36 CIE XYZ color space endpoints
    uint32_t bV5GammaRed     ; // 0x5C          4 Gamma red component
    uint32_t bV5GammaGreen   ; // 0x60          4 Gamma green component
    uint32_t bV5GammaBlue    ; // 0x64          4 Gamma blue component
    uint32_t bV5Intent       ; // 0x68          4 Rendering intent
    uint32_t bV5ProfileData  ; // 0x6C          4 Offset to ICC profile data
    uint32_t bV5ProfileSize  ; // 0x70          4 Size of embedded profile data
    uint32_t bV5Reserved     ; // 0x74          4 Reserved (must be 0)
};


struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color)
{
    //allocate the memory
    struct canvas* canvas_ptr = malloc(sizeof(struct canvas)); 
    if (canvas_ptr == NULL){return NULL;}

    //set the given values of parameters and others as NULL to prevent grabage values 
    canvas_ptr->height = height; 
    canvas_ptr->width = width; 
    canvas_ptr->background = background_color; 
    canvas_ptr->top = NULL; 
    canvas_ptr->bottom = NULL; 

    return canvas_ptr; 
}

struct sprite* animate_create_sprite(const char* file) {

    if (file == NULL) return NULL;

    //open bitmap file for reading 
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) return NULL;

    struct bitmap_header bh;
    if (fread(&bh, sizeof(struct bitmap_header), 1, fp) != 1) {
        fclose(fp);
        return NULL;
    }

    struct bitmapv5_header v5;
    if (fread(&v5, sizeof(struct bitmapv5_header), 1, fp) != 1) {
        fclose(fp);
        return NULL;
    }

    // Reject files that are not the supported 32-bit Bitmap V5 format.
    if (bh.magic[0] != 'B' || bh.magic[1] != 'M' ||
        v5.bV5Size != sizeof(struct bitmapv5_header) ||
        v5.bV5Planes != 1 || v5.bV5BitCount != 32 ||
        (v5.bV5Compression != 0 && v5.bV5Compression != 3)) {
        fclose(fp);
        return NULL;
    }

    size_t width  = v5.bV5Width;
    size_t height = v5.bV5Height;

    if (width == 0 || height == 0 ||
        width > SIZE_MAX / height ||
        width * height > SIZE_MAX / sizeof(color_t)) {
        fclose(fp);
        return NULL;
    }

    //create memory for sprite and initialise its fields 
    struct sprite* sp = malloc(sizeof(struct sprite));
    if (sp == NULL) {
        fclose(fp);       // sp is null so only file needs closing
        return NULL;
    }
    sp->width     = width;
    sp->height    = height;
    sp->ref_count = 0;

    //create memory for pixel 
    sp->pixels = malloc(width * height * sizeof(color_t));
    if (sp->pixels == NULL) {
        fclose(fp);
        free(sp);
        return NULL;
    }

    //Seek to pixel data and read rows in reverse to correct BMP's bottom-up storage order
    if (fseek(fp, (long)bh.pixel_offset, SEEK_SET) != 0) {
        fclose(fp);
        free(sp->pixels);
        free(sp);
        return NULL;
    }

    for (size_t row = 0; row < height; row++) {
        size_t dest_row = height - 1 - row;
        if (fread(&sp->pixels[dest_row * width], sizeof(color_t), width, fp) != width) {
            fclose(fp);
            free(sp->pixels);
            free(sp);
            return NULL;
        }
    }

    fclose(fp);
    return sp;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) {

    // The public API reserves this argument for future outline support.
    (void)filled;

    // Allocate memory in sprite 
    struct sprite* circle_ptr = malloc(sizeof(struct sprite)); 
    if (circle_ptr == NULL) { return NULL; } 
    circle_ptr->ref_count = 0;

    //Diameter is 2r+1 so the centre pixel fits exactly in middle 
    size_t diameter = 2*radius+1; 
    circle_ptr->height = diameter; 
    circle_ptr->width = diameter; 
    size_t num_of_pixels = diameter * diameter; 

    //Allocate memory for pixel array 
    circle_ptr->pixels = malloc(num_of_pixels * sizeof(color_t)); 
    if (circle_ptr->pixels == NULL) {
        free(circle_ptr);
        return NULL;
    }

    //Fill pixels using the circle equation x²+y² <= r²; set others transparent
    for (size_t row = 0; row < diameter; row++) {
        for (size_t col = 0; col < diameter; col++) {
            // offset from centre (signed)
            ssize_t x = (ssize_t)col - (ssize_t)radius;
            ssize_t y = (ssize_t)row - (ssize_t)radius;
            if (x*x + y*y <= (ssize_t)(radius*radius)) {
                circle_ptr->pixels[row * diameter + col] = c;  
            } else {
                circle_ptr->pixels[row * diameter + col] = 0;  // alpha=0, transparent
            }
        }
    }
    
    return circle_ptr; 
}

struct sprite* animate_create_rectangle(size_t width, size_t height,
                                        color_t c, bool filled){
    //create memory for the struct sprite 
    struct sprite* ptr = malloc(sizeof(struct sprite)); 
    if (ptr == NULL){ return NULL; }
   
    ptr->height = height; 
    ptr->width = width; 
    ptr->ref_count = 0; 

    //Allocate memory for the pixel(array)
    ptr->pixels = malloc(height * width * sizeof(color_t));
    if (ptr->pixels == NULL){ 
        free(ptr);
        return NULL; }
    
    // Fill border pixels with colour; fill interior only if filled=true, else transparent
    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            bool is_border = (row == 0 || row == height-1 || col == 0 || col == width-1);
            if (filled || is_border) {
                ptr->pixels[row * width + col] = c;
            } else {
                ptr->pixels[row * width + col] = 0; // transparent
            }
        }
    }
    return ptr; 
    
}

bool animate_destroy_sprite(struct sprite* sprite) {
    if (sprite == NULL) return 1; 
    // Refuse to free a sprite that is still in use by a placement
    if( sprite->ref_count > 0){ 
        return 1; 
    }
    free(sprite->pixels); 
    free(sprite); 
    return 0;
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas,
                                              struct sprite* sprite,
                                              ssize_t x, ssize_t y) {
    if (canvas == NULL || sprite == NULL) return NULL;                                             

    // Allocate memory for the placement and initialise its fields
    struct sprite_placement* sp_ptr = malloc(sizeof(struct sprite_placement));
    if (sp_ptr == NULL) return NULL;
    
    sp_ptr->sprite = sprite;
    sp_ptr->canvas = canvas;
    sp_ptr->x = x;
    sp_ptr->y = y;
    sp_ptr->vx = 0;   
    sp_ptr->vy = 0;
    sp_ptr->ax = 0;
    sp_ptr->ay = 0;
    sp_ptr->next = NULL;
    sp_ptr->previous = NULL;

    if (canvas->top == NULL) {
        // empty canvas — new placement is both top and bottom
        canvas->top    = sp_ptr;   
        canvas->bottom = sp_ptr;
    } else {
        // Attach new placement at the top of the list
        canvas->top->next = sp_ptr;     
        sp_ptr->previous  = canvas->top; 
        canvas->top       = sp_ptr;      
    }

    sprite->ref_count++;
    return sp_ptr;
}

void animate_placement_up(struct sprite_placement* sprite_placement){

    //case 1: already at top — do nothing
    if (sprite_placement->next == NULL) return;

    //save all neighbours before relinking 
    struct sprite_placement* above = sprite_placement->next;   
    struct sprite_placement* below = sprite_placement->previous; 
    struct sprite_placement* above_above = above->next; 
    struct canvas* c = sprite_placement->canvas;

    //Relink 
    if (below != NULL) {
        below->next = above;
    } else {  
        c->bottom = above;
    }

    above->previous = below;
    above->next = sprite_placement;
    sprite_placement->previous = above;
    sprite_placement->next = above_above;

    if (above_above != NULL) {
        above_above->previous = sprite_placement;
    } else {
        c->top = sprite_placement;
    }

}

void animate_placement_down(struct sprite_placement* sprite_placement){
    // case 1: already at bottom — do nothing
    if (sprite_placement->previous == NULL) return;

    // save all neighbours before relinking
    struct sprite_placement* below = sprite_placement->previous;       
    struct sprite_placement* above = sprite_placement->next;          
    struct sprite_placement* below_below = below->previous; 
    struct canvas* c = sprite_placement->canvas;

    if (above != NULL) {
        above->previous = below;
    } else {
        c->top = below;
    }

    below->next = above;
    below->previous = sprite_placement;
    sprite_placement->next = below;
    sprite_placement->previous = below_below;

    if (below_below != NULL) {
        below_below->next = sprite_placement;
    } else {
        c->bottom = sprite_placement;
    }

}

void animate_placement_top(struct sprite_placement* sprite_placement){
    // already at top — do nothing
    if (sprite_placement->next == NULL) return;

    struct canvas* c = sprite_placement->canvas;

    //Unlink sp from wherever it is
    if (sprite_placement->previous == NULL) {
        // sp is at bottom
        c->bottom = sprite_placement->next;
        sprite_placement->next->previous = NULL;
    } else {
        // sp is in the middle
        sprite_placement->previous->next = sprite_placement->next;
        sprite_placement->next->previous = sprite_placement->previous;
    }
    //Attach sp at the top
    sprite_placement->previous = c->top;
    sprite_placement->next = NULL;
    c->top->next = sprite_placement;
    c->top = sprite_placement;
}

void animate_placement_bottom(struct sprite_placement* sprite_placement){
        // already at bottom — do nothing
    if (sprite_placement->previous == NULL) return;

    struct canvas* c = sprite_placement->canvas;

    //Unlink sp from wherever it is
    if (sprite_placement->next == NULL) {
        // sp is at top
        c->top               = sprite_placement->previous;
        sprite_placement->previous->next   = NULL;
    } else {
        // sp is in the middle
        sprite_placement->previous->next = sprite_placement->next;
        sprite_placement->next->previous   = sprite_placement->previous;
    }

    //Attach sp at the bottom
    sprite_placement->next = c->bottom;
    sprite_placement->previous = NULL;
    c->bottom->previous = sprite_placement;
    c->bottom = sprite_placement;
}

void animate_destroy_placement(struct sprite_placement* sp) {
    struct canvas* c = sp->canvas;

    // case 1: only node in list
    if (sp->previous == NULL && sp->next == NULL) {
        c->top    = NULL;
        c->bottom = NULL;

    // case 2: at the bottom (no previous, has next)
    } else if (sp->previous == NULL && sp->next != NULL) {
        c->bottom          = sp->next;
        sp->next->previous = NULL;

    // case 3: at the top (has previous, no next)
    } else if (sp->previous != NULL && sp->next == NULL) {
        c->top             = sp->previous;
        sp->previous->next = NULL;

    // case 4: in the middle
    } else {
        sp->previous->next     = sp->next;
        sp->next->previous     = sp->previous;
    }

    sp->sprite->ref_count--;
    free(sp);
}

void animate_set_animation_params(struct sprite_placement* sprite_placement,
                                  ssize_t vx, ssize_t vy,
                                  ssize_t ax, ssize_t ay){
    sprite_placement->vx = vx; 
    sprite_placement->vy = vy; 
    sprite_placement->ax = ax; 
    sprite_placement->ay = ay; 
}

void animate_destroy_canvas(struct canvas* canvas) {
    struct sprite_placement* current = canvas->bottom;  // initialise before loop
    while (current != NULL) {
        struct sprite_placement* next = current->next;  // save next FIRST
        animate_destroy_placement(current);              // destroys current
        current = next;                                  // move to saved next
    }
    free(canvas);
}

size_t animate_frame_size_bytes(struct canvas* canvas){
    return (canvas->height * canvas->width * sizeof(color_t));
}

void animate_generate_frame(const struct canvas* canvas, size_t frame,
                            size_t frame_rate, void* buf) {

    // cast buf and fill every pixel with background colour
    color_t* pixels = (color_t*)buf;
    size_t total = canvas->width * canvas->height;
    for (size_t i = 0; i < total; i++) {
        pixels[i] = canvas->background|0xFF000000;
    }

    float t = (float)frame / (float)frame_rate;

    // walk linked list from bottom to top
    struct sprite_placement* current = canvas->bottom;
    while (current != NULL) {

        struct sprite* s = current->sprite;
        ssize_t actual_x = current->x + (ssize_t)(current->vx * t + current->ax * t * t / 2.0f);    
        ssize_t actual_y = current->y + (ssize_t)(current->vy * t + current->ay * t * t / 2.0f);
        // loop over every pixel in this sprite
        for (size_t sr = 0; sr < s->height; sr++) {
            for (size_t sc = 0; sc < s->width; sc++) {

                // compute where this sprite pixel lands on canvas
                ssize_t canvas_row = actual_y + (ssize_t)sr;
                ssize_t canvas_col = actual_x + (ssize_t)sc;

                // skip if outside canvas bounds
                if (canvas_row < 0 || canvas_row >= (ssize_t)canvas->height) continue;
                if (canvas_col < 0 || canvas_col >= (ssize_t)canvas->width)  continue;

                // get this sprite pixel
                color_t pixel = s->pixels[sr * s->width + sc];

                // skip if transparent (alpha == 0)
                if ((pixel >> 24) == 0) continue;

                // write to buffer, forcing alpha to 0xFF
                pixels[canvas_row * (ssize_t)canvas->width + canvas_col] = pixel | 0xFF000000;
            }
        }

        // move to next placement (toward top)
        current = current->next;
    }
}
   

// Optional extension
void animate_set_animation_function(struct sprite_placement* sprite_placement,
                                    animate_fn fn, void* priv) {
    // Reserved extension point; custom callbacks are not implemented yet.
    (void)sprite_placement;
    (void)fn;
    (void)priv;
}
