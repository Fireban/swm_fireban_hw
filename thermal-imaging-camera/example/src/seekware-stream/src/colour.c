/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:     Seek Thermal SDK demo
 * Purpose:     Pixel handling for coloured images
 * Author:      Seek Thermal, Inc.
 * License:     Proprietary
 * Comments:    
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "colour.h"

#include <linux/fb.h>
#include <math.h>

extern struct fb_var_screeninfo vinfo;
extern struct fb_fix_screeninfo finfo;
extern int bpp;

#define COLOUR_RESOLUTION   1024

static union pix colour_table[COLOUR_RESOLUTION];
static float conversion[65536];

int putPixel16(int x, int y, uint16_t pixel)
{
    return putPixelFloat(x, y, conversion[pixel]);
}

int putPixelDirect(int x, int y, const union imgdata * pixel)
{
    union pix * dest = (union pix *)(pixels + (x+vinfo.xoffset) * bpp + (y+vinfo.yoffset) * finfo.line_length);

    switch (vinfo.bits_per_pixel) {
        case 16:
            dest->p16.r = pixel->r;
            dest->p16.g = pixel->g;
            dest->p16.b = pixel->b;
        break;
        case 24:
            dest->p24.r = pixel->r;
            dest->p24.g = pixel->g;
            dest->p24.b = pixel->b;
        break;
        case 32:
            dest->p32.r = pixel->r;
            dest->p32.g = pixel->g;
            dest->p32.b = pixel->b;
        break;
        default:
            return -1;
        break;
    };

    return 0;
}

int putPixelFloat(int x, int y, float pixel)
{
    int ipx = (int)(pixel * (float)COLOUR_RESOLUTION);
    if (ipx < 0) {
        ipx = 0;
    } else if (ipx >= COLOUR_RESOLUTION) {
        ipx = COLOUR_RESOLUTION-1;
    }
    const union pix * src = colour_table + ipx;
    union pix * dest = (union pix *)(pixels + (x+vinfo.xoffset) * bpp + (y+vinfo.yoffset) * finfo.line_length);

    switch (vinfo.bits_per_pixel) {
        case 16:
            dest->p16 = src->p16;
        break;
        case 24:
            dest->p24 = src->p24;
        break;
        case 32:
            dest->p32 = src->p32;
        break;
        default:
            return -1;
        break;
    };

    return 0;
}

int initColours(void)
{
    const int GREEN_1    =   (COLOUR_RESOLUTION * 1) / 4;
    const int GREEN_C    =   (COLOUR_RESOLUTION * 2) / 4;
    const int GREEN_2    =   (COLOUR_RESOLUTION * 3) / 4;
    const int BLUE_1     =   (COLOUR_RESOLUTION * 1) / 4;
    const int BLUE_2     =   (COLOUR_RESOLUTION * 7) / 12;
    const int BLUE_3     =   (COLOUR_RESOLUTION * 3) / 4;
    const int RED_1      =   (COLOUR_RESOLUTION * 5) / 12;
    const int RED_2      =   (COLOUR_RESOLUTION * 3) / 4;
    const float pi_g     =   M_PI/(GREEN_2-GREEN_1);
    const float pi_b     =   M_PI/(2*(BLUE_2-BLUE_1));
    const float pi_r     =   M_PI/(2*(RED_2-RED_1));
    int i, r, g, b;

    for (i = 0; i < COLOUR_RESOLUTION; ++i) {
        union pix * colour = colour_table + i;
        if (i < GREEN_1) {
            g = 0;
        } else if (i < GREEN_2) {
            float alpha = pi_g * (i-GREEN_C);
            g = 0.66 * 255.5 * cosf(alpha);
        } else {
            g = (i-GREEN_2) * (255.5/GREEN_1);
        }
        if (i < BLUE_1) {
            b = i * (255.5/BLUE_1);
        } else if (i < BLUE_2) {
            float alpha = pi_b * (i-BLUE_1);
            b = 255.5 * cosf(alpha);
        } else if (i < BLUE_3) {
            b = 0;
        } else {
            b = (i-BLUE_3) * (255.5/BLUE_1);
        }
        if (i < RED_1) {
            r = 0;
        } else if (i < RED_2) {
            float alpha = pi_r * (i-RED_1);
            r = 255.5 * sinf(alpha);
        } else {
            r = 255;
        }
        switch (vinfo.bits_per_pixel) {
            case 16:
                colour->p16.r = r / 8;
                colour->p16.g = g / 4;
                colour->p16.b = b / 8;
            break;
            case 24:
                colour->p24.r = r;
                colour->p24.g = g;
                colour->p24.b = b;
            break;
            case 32:
                colour->p32.r = r;
                colour->p32.g = g;
                colour->p32.b = b;
            break;
            default:
                return -1;
            break;
        }
    }

    for (i = 0; i <= 0xffff; ++i) {
        static const float bits[] = {
                      32.0,    64.0,   128.0,
             256.0,  512.0,  1024.0,  2048.0,
            4096.0, 8192.0, 16384.0, 32768.0,
               0.0,    0.0,     0.0,     0.0,
               0.0
        };
        int j;
        float r = 0.0f;
        for (j=0; j<16; ++j) {
            int m = 1 << j;
            if (i & m) {
                r += bits[j];
            }
        }
        conversion[i] = r / 65536.0f;
    }

    return 0;
}

/* * * * * * * * * * * * * End - of - File * * * * * * * * * * * * * * */
