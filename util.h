#ifndef __UTIL_H
#define __UTIL_H

#include "common.h"
#include "termio.h"

#define pix_at(img, x, y) (img->pixels[(y) * img->width + (x)])

typedef enum {
  IMGT_JPEG,
  IMGT_PNG,
  IMGT_WEBP,
  IMGT_UNKNOWN
} img_type_t;

typedef struct {
  unsigned int height, width;
  colour_t *pixels;
} image_t;

img_type_t guess_image_type(unsigned char *raw_img, unsigned long length);
int decode_image(image_t* out, unsigned char *raw_img, unsigned long length, img_type_t image_type);
int decode_jpeg(image_t* out, unsigned char *raw_img, unsigned long length);
int decode_png(image_t* out, unsigned char *raw_img, unsigned long length);

int print_image(int fildes, image_t *img);

int swrite(int fildes, const char* msg);

// Time elapsed between t1 (newer) and t2 (older)
int elapsed_ms(struct timespec *t1, struct timespec *t2);

#endif
