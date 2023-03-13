#ifndef __UTIL_H
#define __UTIL_H

#include "common.h"

typedef enum {
  IMGT_JPEG,
  IMGT_PNG,
  IMGT_UNKNOWN
} img_type_t;

typedef struct {
  unsigned char r, g, b;
} pixel_t;

typedef struct {
  unsigned int height, width;
  pixel_t *pixels;
} image_t;

img_type_t guess_image_type(unsigned char *raw_img, unsigned long length);
int decode_image(image_t* out, unsigned char *raw_img, unsigned long length, img_type_t image_type);
int decode_jpeg(image_t* out, unsigned char *raw_img, unsigned long length);
int decode_png(image_t* out, unsigned char *raw_img, unsigned long length);

int swrite(int fildes, const char* msg);

#endif
