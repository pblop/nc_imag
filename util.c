#include "util.h"

#include "termio.h"
#include "lodepng/lodepng.h"

#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
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
  if (length >= 4 && raw_img[0] == 0x89 && raw_img[1] == 'P' &&
                     raw_img[2] == 'N' && raw_img[3] == 'G')
    return IMGT_PNG;
  if (length >= 12 && raw_img[0] == 'R'  && raw_img[1] == 'I' &&
                      raw_img[2] == 'F'  && raw_img[3] == 'F' &&
                      raw_img[8] == 'W'  && raw_img[9] == 'E' &&
                      raw_img[10] == 'B' && raw_img[11] == 'P')
    return IMGT_WEBP;

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

int decode_jpeg(image_t* out, unsigned char *jpg_buffer, unsigned long jpg_size)/*{{{*/
{
  // Basically a copy of this https://gist.github.com/PhirePhly/3080633
  int rc;

	// Variables for the decompressor itself
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	// Variables for the output buffer, and how long each row is
	unsigned long bmp_size;
	unsigned char *bmp_buffer;
	int row_stride, pixel_size;

  cinfo.err = jpeg_std_error(&jerr);	
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);
  rc = jpeg_read_header(&cinfo, TRUE);

	if (rc != 1)
    return -1;

  jpeg_start_decompress(&cinfo);

	out->width = cinfo.output_width;
	out->height = cinfo.output_height;
	pixel_size = cinfo.output_components;

	bmp_size = out->width * out->height * pixel_size;
	bmp_buffer = (unsigned char*) malloc(bmp_size);

  row_stride = out->width * pixel_size;

	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + \
						   (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  // NOTE: We don't free the input buffer because that's main.c territory.
  // We don't care whether it was dinamically or statically allocated.

  // TODO: Do this in a more elegant way, maybe inside the while loop.
  // Allocate memory for our actual image.
  out->pixels = malloc(out->width * out->height * sizeof(colour_t));
  for (unsigned int i = 0; i*pixel_size < bmp_size; i++)
  {
    out->pixels[i] = (colour_t) {
      .r = bmp_buffer[i*pixel_size],
      .g = bmp_buffer[i*pixel_size+1],
      .b = bmp_buffer[i*pixel_size+2],
      .a = 0
    };
  }

  free(bmp_buffer);

  return 0;
}/*}}}*/

int decode_png(image_t* out, unsigned char *raw_img, unsigned long length)/*{{{*/
{
  unsigned int error;
  unsigned char *lodeimg = 0;

  error = lodepng_decode32(&lodeimg, &out->width, &out->height, raw_img, length);
  if (!error)
    out->pixels = (colour_t*) lodeimg;

  return error;
}/*}}}*/

int print_image(int fildes, image_t *img)/*{{{*/
{
  colour_t *ptop, *pbot, *pptop, *ppbot;
  ptop = pbot = pptop = ppbot = NULL;

  swrite(fildes, CLEAR GOTO_HOME);
  for (unsigned int y = 0; y < img->height; y+=2)
  {
    dprintf(fildes, GOTO, y/2+1, 1);
    for (unsigned int x = 0; x < img->width; x++)
    {
      ptop = &pix_at(img, x, y);
      if ((y+1) >= img->height)
        pbot = NULL; // If we're on the last row, just print the top pixel.
      else
        pbot = &pix_at(img, x, y+1);

      // If the previous pixel was the same colour, don't print that
      // extra colour code.
      if (!colour_eq_na(pptop, ptop))
        dprint_colour(fildes, ptop, POSITION_FOREGROUND);
      if (!colour_eq_na(ppbot, pbot))
        dprint_colour(fildes, pbot, POSITION_BACKGROUND);

      swrite(fildes, "▀");

      // Save previous (current) pixel for next iteration
      pptop = ptop;
      ppbot = pbot;
    }
  }
  dprint_colour(fildes, NULL, POSITION_FOREGROUND);
  dprint_colour(fildes, NULL, POSITION_BACKGROUND);

  return 0;
}/*}}}*/

int elapsed_ms(struct timespec *t1, struct timespec *t2)/*{{{*/
{
  return (t1->tv_sec - t2->tv_sec) * 1000 + (t1->tv_nsec - t2->tv_nsec) / 1000000;  
}/*}}}*/

