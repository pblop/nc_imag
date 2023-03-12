#include "common.h"
#include "termio.h"
#include "util.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>

struct {
  int sockfd;
} globals = {-1};

void sigint_handler(int signum);
void setup_sigint_handler(void);

int main()
{
  setup_sigint_handler();

  // Create the socket
  if ((globals.sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("create socket");
    return 1;
  }
  


  return 0;
}

void sigint_handler(int signum)/*{{{*/
{
  UNUSED(signum);

  if (globals.sockfd != -1)
    shutdown(globals.sockfd, SHUT_RDWR); // only should fail if our socket
                                         // is already closed.
  

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
