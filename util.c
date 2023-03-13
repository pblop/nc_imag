#include "util.h"

#include "lodepng/lodepng.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int swrite(int fildes, const char* msg)
{
  return write(fildes, msg, strlen(msg));
}

img_type_t guess_image_type(unsigned char *raw_img, unsigned long length)/*{{{*/
{
  if (length >= 4 && raw_img[0] == 0xff && raw_img[1] == 0xd8 &&
                     raw_img[2] == 0xff && raw_img[3] == 0xe0)
    return IMGT_JPEG;
  if (length >= 4 && raw_img[0] == 0x89 && raw_img[1] == 0x50 &&
                     raw_img[2] == 0x4e && raw_img[3] == 0x47)
    return IMGT_PNG;

  return IMGT_UNKNOWN;
}/*}}}*/

int decode_image(image_t* out, unsigned char *raw_img, unsigned long length, img_type_t image_type)/*{{{*/
{
  switch (image_type)
  {
    case IMGT_JPEG:
      return decode_jpeg(out, raw_img, length);
    case IMGT_PNG:
      return decode_png(out, raw_img, length);

    default:
      return -1;
  }
}/*}}}*/

int decode_jpeg(image_t* out, unsigned char *raw_img, unsigned long length)/*{{{*/
{
  UNUSED(out);
  UNUSED(raw_img);
  UNUSED(length);
  return -1;
}/*}}}*/

int decode_png(image_t* out, unsigned char *raw_img, unsigned long length)/*{{{*/
{
  unsigned int error;

  error = lodepng_decode32((unsigned char**) out->pixels, &out->width, &out->height, raw_img, length);
  return error;
}/*}}}*/

