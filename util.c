#include "util.h"

#include <string.h>
#include <unistd.h>

int swrite(int fildes, const char* msg)
{
  return write(fildes, msg, strlen(msg));
}
