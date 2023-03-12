#include "termio.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>

struct termios orig_screen_attrs;
clock_t frame_start;

// Copied this fn from
// https://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input
int kbhit(void)/*{{{*/
{
  struct timeval tv = {0L, 0L};

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv) > 0;
}/*}}}*/
termkey_t get_user_input(void)/*{{{*/
{
  int c;

  if (!kbhit())
    return KEY_NONE;

  if ((c = getc(stdin)) == EOF)
    return -12;

  switch (c)
  {
    case 'a': case 'A': return KEY_A;
    case 'b': case 'B': return KEY_B;
    case 'c': case 'C': return KEY_C;
    case 'd': case 'D': return KEY_D;
    case 'e': case 'E': return KEY_E;
    case 'f': case 'F': return KEY_F;
    case 'g': case 'G': return KEY_G;
    case 'h': case 'H': return KEY_H;
    case 'i': case 'I': return KEY_I;
    case 'j': case 'J': return KEY_J;
    case 'k': case 'K': return KEY_K;
    case 'l': case 'L': return KEY_L;
    case 'm': case 'M': return KEY_M;
    case 'n': case 'N': return KEY_N;
    case 'o': case 'O': return KEY_O;
    case 'p': case 'P': return KEY_P;
    case 'q': case 'Q': return KEY_Q;
    case 'r': case 'R': return KEY_R;
    case 's': case 'S': return KEY_S;
    case 't': case 'T': return KEY_T;
    case 'u': case 'U': return KEY_U;
    case 'v': case 'V': return KEY_V;
    case 'w': case 'W': return KEY_W;
    case 'x': case 'X': return KEY_X;
    case 'y': case 'Y': return KEY_Y;
    case 'z': case 'Z': return KEY_Z;

    case 27: // ansi escape code for arrows
      /*{{{*/
      if ((c = getc(stdin)) == EOF)
        return KEY_NONE;
      if (c == 91)// continuation of the ansi escape code for arrows
      {
        if ((c = getc(stdin)) == EOF)
          return KEY_NONE;

        switch (c)
        {
          case 'A':
            return KEY_UP;
          case 'B':
            return KEY_DOWN;
          case 'C':
            return KEY_RIGHT;
          case 'D':
            return KEY_LEFT;
        }
      }

      break;/*}}}*/

    case 195: // utf-8 stuff (ñ)
      /*{{{*/
      if ((c = getc(stdin)) == EOF)
        return KEY_NONE;
      if (c == 177)// continuation of the ansi escape code for ñ
      {
        if ((c = getc(stdin)) == EOF)
          return KEY_NONE;

        if (c == 10)
          return KEY_Nn;
      }

      break;/*}}}*/
  }
  return KEY_NONE;
}/*}}}*/

int setup_screen(void)/*{{{*/
{
  struct termios tattr;
  
  // set noncanonical input mode
  if (!isatty(STDIN_FILENO))
  {
    fprintf(stderr, "Not a terminal");
    return -1;
  }

  // Save the terminal attributes so we can restore them later.
  tcgetattr(STDIN_FILENO, &orig_screen_attrs);

  // Set the funny terminal modes.
  tcgetattr(STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
  
  return 0;
}/*}}}*/
void unsetup_screen(void)/*{{{*/
{
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_screen_attrs);
  printf(SHOW_CURSOR REMOVE_COLOUR REMOVE_BACKGROUND CLEAR GOTO_HOME);
  fflush(stdout);
}
/*}}}*/

int ensure_screen_size(void)/*{{{*/
{
  int width, height, prevwidth, prevheight;

  prevwidth = 0, prevheight = 0;
  if (get_window_size(&width, &height) != 0)
    return -1;

  while (width < SCREEN_WIDTH || height < SCREEN_HEIGHT)
  {
    if (prevwidth != width || prevheight != height)
    {
      printf(CLEAR GOTO
          "Screen size is too small.\n"
          "Please resize your terminal to at least %dx%d.\n"
          "The current size is %dx%d.\n",
          1, 1,
          SCREEN_WIDTH, SCREEN_HEIGHT, width, height);
      fflush(stdout);
    }

    prevwidth = width, prevheight = height;
    if (get_window_size(&width, &height) != 0)
      return -1;
  }

  return 0;
}/*}}}*/
int get_window_size(int *width, int *height)/*{{{*/
{
  struct winsize ws;
  if (ioctl(0, TIOCGWINSZ, &ws) != 0)
  {
    perror("ioctl");
    return -1;
  }

  *width = ws.ws_col;
  *height = ws.ws_row;
  return 0;
}/*}}}*/

int msleep(long msec)/*{{{*/
{
  struct timespec ts;
  int res;

  if (msec < 0)
    return -1;

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000L;

  do {
    res = nanosleep(&ts, &ts);
  } while (res);

  return res;
}/*}}}*/
void start_frame(void)/*{{{*/
{
  frame_start = clock();
}/*}}}*/
void ensure_frame_time(int ms)/*{{{*/
{
  double elapsed = (double)(clock() - frame_start) / (CLOCKS_PER_SEC * 1000);
  if (elapsed < ms)
    msleep(ms - elapsed);
}/*}}}*/

