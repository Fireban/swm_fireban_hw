/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:     Seek Thermal SDK demo
 * Purpose:     Pixel handling for coloured images
 * Author:      Seek Thermal, Inc.
 * License:     Proprietary
 * Comments:    
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __UTILS_SEEKTHERMAL_TEST_COLOUR_H_INCLUDED__
#define __UTILS_SEEKTHERMAL_TEST_COLOUR_H_INCLUDED__

#include <stdint.h>

/// The frame buffer device memory
extern uint8_t * pixels;

/// Pixel format in the framebuffer device
union pix {
    struct {
        uint16_t    b:5;
        uint16_t    g:6;
        uint16_t    r:5;
    } p16;
    struct {
        uint32_t    b:8;
        uint32_t    g:8;
        uint32_t    r:8;
    } p24;
    struct {
        uint32_t    b:8;
        uint32_t    g:8;
        uint32_t    r:8;
        uint32_t    a:8;
    } p32;
} __attribute__((packed));

/// Pixel format in the RGB image buffer
union imgdata {
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
    uint32_t raw;
} __attribute__((packed));

extern int putPixel16(int x, int y, uint16_t pixel);
extern int putPixelFloat(int x, int y, float pixel);
extern int putPixelDirect(int x, int y, const union imgdata * pixel);
extern int initColours(void);

#endif /* __UTILS_SEEKTHERMAL_TEST_COLOUR_H_INCLUDED__ */

/* * * * * * * * * * * * * End - of - File * * * * * * * * * * * * * * */
