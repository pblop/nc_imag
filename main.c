#include "common.h"
#include "termio.h"
#include "util.h"
#include <stdio.h>

void sigint_handler(int signum);
void setup_sigint_handler(void);

void sigint_handler(int signum)/*{{{*/
{
  UNUSED(signum);

  exit(0);
}/*}}}*/
void setup_sigint_handler(void)/*{{{*/
{
  sigset_t empty_set;
  struct sigaction action;

  // Capture SIGINT and exit program safely then.
  sigemptyset(&empty_set);
  action.sa_handler = sigint_handler;
  action.sa_mask = empty_set;
  action.sa_flags = SA_RESTART;
  sigaction(SIGINT, &action, NULL);
}/*}}}*/

int main()
{
  return 0;
}
