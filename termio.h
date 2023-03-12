#ifndef __TERMIO_H
#define __TERMIO_H

#include "common.h"
#include <termios.h>
#include <time.h>

#define CSI "\x1b["
#define GOTO CSI"%d;%dH"
#define GOTO_HOME CSI"H"
#define CLEAR CSI"2J"
#define CLEAR_TO_HOME CSI"1J"
#define CLEAR_LINE_TO_START CSI"1K"
#define HIDE_CURSOR CSI"?25l"
#define SHOW_CURSOR CSI"?25h"
#define RGB_COLOUR CSI"38;2;%d;%d;%dm"
#define REMOVE_COLOUR CSI"39m"
#define RGB_BACKGROUND CSI"48;2;%d;%d;%dm"
#define REMOVE_BACKGROUND CSI"49m"
#define UNDERLINE_ON CSI"4m"
#define UNDERLINE_OFF CSI"24m"

extern struct termios orig_screen_attrs;
extern clock_t frame_start;

typedef struct {
  unsigned char r, g, b;
} colour_t;

// KB HANDLING /*{{{*/
typedef enum
{
  KEY_NONE = -1,
  KEY_UP = 0,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_Nn, // ñ

  KEY_A = 97,
  KEY_B = 98,
  KEY_C = 99,
  KEY_D = 100,
  KEY_E = 101,
  KEY_F = 102,
  KEY_G = 103,
  KEY_H = 104,
  KEY_I = 105,
  KEY_J = 106,
  KEY_K = 107,
  KEY_L = 108,
  KEY_M = 109,
  KEY_N = 110,
  KEY_O = 111,
  KEY_P = 112,
  KEY_Q = 113,
  KEY_R = 114,
  KEY_S = 115,
  KEY_T = 116,
  KEY_U = 117,
  KEY_V = 118,
  KEY_W = 119,
  KEY_X = 120,
  KEY_Y = 121,
  KEY_Z = 122
} termkey_t;

int kbhit();
termkey_t get_user_input();
/*}}}*/

int setup_screen(void);
void unsetup_screen(void);

int ensure_screen_size(void);
int get_window_size(int *width, int *height);

int msleep(long msec);
void start_frame(void);
void ensure_frame_time(int ms);

#endif
