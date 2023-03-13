#include <stdio.h>
#include <stdlib.h>
#include "util.h"

#define INPUT_BUFSIZE 1000*1000

int main(void)
{
  FILE* f;
  unsigned char buf[INPUT_BUFSIZE+1];
  size_t len;
  image_t img = {0, 0, NULL};

  if ((f = fopen("test.png", "r")) == NULL)
  {
    perror("fopen");
    return 1;
  }

  fread(&buf, INPUT_BUFSIZE+1, 1, f);
  if (feof(f))
    len = ftell(f);
  else
  {
    fprintf(stderr, "File too large\n");
    return 1;
  }

  printf("file size: %zu\n", len);
  if (decode_png(&img, buf, len) != 0)
  {
    fprintf(stderr, "Error while decoding\n");
    return 1;
  }
  printf("image data: width: %u | height: %u\n", img.width, img.height);
  //for (int i = 0; i < img.width*img.height; i++)
  //  printf("image data: r: %x | g: %x | b: %x\n", img.pixels[0].r, img.pixels[0].g, img.pixels[0].b);

  print_image(0, &img);
  //free(img.pixels);

  return 0;
}
