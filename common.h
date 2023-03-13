#ifndef __COMMON_H
#define __COMMON_H

#include <stdbool.h>
#define _POSIX_C_SOURCE 200809L
// this is for when I'm not using a variable but I don't want the compiler
// nagging about it being unused.
#define UNUSED(x) (void)(x)
// this is for fancy for (EVER) loops
#define EVER ;;

#endif
