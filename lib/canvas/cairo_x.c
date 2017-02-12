/*
Code from node-canvas (https://github.com/Automattic/node-canvas)

Copyright (c) 2010 LearnBoost, and contributors <dev@learnboost.com>

Copyright (c) 2014 Automattic, Inc and contributors <dev@automattic.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the 'Software'), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cairo/cairo.h>
#include <stdlib.h>
#include <stdint.h>

void blur(cairo_surface_t *surface, int radius) {
    // Steve Hanov, 2009
    // Released into the public domain.
    radius = radius * 0.57735f + 0.5f;
    // get width, height
    int width = cairo_image_surface_get_width( surface );
    int height = cairo_image_surface_get_height( surface );
    unsigned* precalc = (unsigned*)malloc(width*height*sizeof(unsigned));
    if (!precalc) {
        return;
    }
    cairo_surface_flush( surface );
    unsigned char* src = cairo_image_surface_get_data( surface );
    double mul=1.f/((radius*2)*(radius*2));
    int channel;

    // The number of times to perform the averaging. According to wikipedia,
    // three iterations is good enough to pass for a gaussian.
    const int MAX_ITERATIONS = 3;
    int iteration;

    for ( iteration = 0; iteration < MAX_ITERATIONS; iteration++ ) {
        for( channel = 0; channel < 4; channel++ ) {
            int x,y;

            // precomputation step.
            unsigned char* pix = src;
            unsigned* pre = precalc;

            pix += channel;
            for (y=0;y<height;y++) {
                for (x=0;x<width;x++) {
                    int tot=pix[0];
                    if (x>0) tot+=pre[-1];
                    if (y>0) tot+=pre[-width];
                    if (x>0 && y>0) tot-=pre[-width-1];
                    *pre++=tot;
                    pix += 4;
                }
            }

            // blur step.
            pix = src + (int)radius * width * 4 + (int)radius * 4 + channel;
            for (y=radius;y<height-radius;y++) {
                for (x=radius;x<width-radius;x++) {
                    int l = x < radius ? 0 : x - radius;
                    int t = y < radius ? 0 : y - radius;
                    int r = x + radius >= width ? width - 1 : x + radius;
                    int b = y + radius >= height ? height - 1 : y + radius;
                    int tot = precalc[r+b*width] + precalc[l+t*width] -
                        precalc[l+b*width] - precalc[r+t*width];
                    *pix=(unsigned char)(tot*mul);
                    pix += 4;
                }
                pix += (int)radius * 2 * 4;
            }
        }
    }

    cairo_surface_mark_dirty(surface);
    free(precalc);
}

void get_image_surface_data(cairo_surface_t *surface,
		int sx, int sy, int sw, int sh, uint8_t *dst)
{
  uint8_t *src = cairo_image_surface_get_data(surface);
  int srcStride = cairo_image_surface_get_stride(surface);
  int dstStride = 4 * sw;
  int x, y;

  // Normalize data (argb -> rgba)
  for (y = 0; y < sh; ++y) {
    uint32_t *row = (uint32_t *)(src + srcStride * (y + sy));
    for (x = 0; x < sw; ++x) {
      int bx = x * 4;
      uint32_t *pixel = row + x + sx;
      uint8_t a = *pixel >> 24;
      uint8_t r = *pixel >> 16;
      uint8_t g = *pixel >> 8;
      uint8_t b = *pixel;
      dst[bx + 3] = a;

      // Performance optimization: fully transparent/opaque pixels can be
      // processed more efficiently.
      if (a == 0 || a == 255) {
        dst[bx + 0] = r;
        dst[bx + 1] = g;
        dst[bx + 2] = b;
      } else {
        float alpha = (float)a / 255;
        dst[bx + 0] = (int)((float)r / alpha);
        dst[bx + 1] = (int)((float)g / alpha);
        dst[bx + 2] = (int)((float)b / alpha);
      }

    }
    dst += dstStride;
  }
}

void put_image_surface_data(cairo_surface_t *surface,
        uint8_t *src, int width, int height,
		int sx, int sy,
		int dx, int dy, int rows, int cols)
{
  uint8_t *dst = cairo_image_surface_get_data(surface);
  int srcStride = 4 * width;
  int dstStride = cairo_image_surface_get_stride(surface);
  int x, y;

  src += sy * srcStride + sx * 4;
  dst += dstStride * dy + 4 * dx;
  for (y = 0; y < rows; ++y) {
    uint8_t *dstRow = dst;
    uint8_t *srcRow = src;
    for (x = 0; x < cols; ++x) {
      // rgba
      uint8_t r = *srcRow++;
      uint8_t g = *srcRow++;
      uint8_t b = *srcRow++;
      uint8_t a = *srcRow++;

      // argb
      // performance optimization: fully transparent/opaque pixels can be
      // processed more efficiently.
      if (a == 0) {
        *dstRow++ = 0;
        *dstRow++ = 0;
        *dstRow++ = 0;
        *dstRow++ = 0;
      } else if (a == 255) {
        *dstRow++ = b;
        *dstRow++ = g;
        *dstRow++ = r;
        *dstRow++ = a;
      } else {
        float alpha = (float)a / 255;
        *dstRow++ = b * alpha;
        *dstRow++ = g * alpha;
        *dstRow++ = r * alpha;
        *dstRow++ = a;
      }
    }
    dst += dstStride;
    src += srcStride;
  }
  cairo_surface_mark_dirty_rectangle(surface, dx, dy, cols, rows);
}


