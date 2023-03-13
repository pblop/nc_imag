#include "util.h"

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

image_t* decode_image(unsigned char *raw_img, unsigned long length, img_type_t image_type)/*{{{*/
{
  switch (image_type)
  {
    case IMGT_JPEG:
      return decode_jpeg(raw_img, length);
    case IMGT_PNG:
      return decode_png(raw_img, length);

    default:
      return NULL;
  }
}/*}}}*/

image_t* decode_jpeg(unsigned char *raw_img, unsigned long length)/*{{{*/
{
  return NULL;
}/*}}}*/

image_t* decode_png(unsigned char *raw_img, unsigned long length)/*{{{*/
{
  return NULL;
}/*}}}*/

