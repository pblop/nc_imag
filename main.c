#include "common.h"
#include "termio.h"
#include "util.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct {
  int sockfd;
} globals = {-1};

void sigint_handler(int signum);
void cleanup_exit(int exitn);
void setup_sigint_handler(void);

int main(int argc, char* argv[])
{
  int port;
  struct sockaddr_in addr;

  if (argc == 0)
  {
    fprintf(stderr, "Usage: ncimag <port>\n");
    return 1;
  }
  if (argc < 2){
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 1;
  }

  // TODO: Check if argv[1] really is a number.
  port = atoi(argv[1]);

  setup_sigint_handler();

  // Create the socket
  if ((globals.sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("create socket");
    cleanup_exit(1);
  }
  
  if (setsockopt(globals.sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
  {
    perror("setsockopt");
    cleanup_exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(globals.sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
    perror("bind");
    cleanup_exit(1);
  }

  printf("Listening on port %d", port);
  if (listen(globals.sockfd, 100) == -1)
  {
    perror("listen");
    cleanup_exit(1);
  }

  return 0;
}

void sigint_handler(int signum)/*{{{*/
{
  UNUSED(signum);
  cleanup_exit(0);
}/*}}}*/
void cleanup_exit(int exitn)/*{{{*/
{
  if (globals.sockfd != -1)
    shutdown(globals.sockfd, SHUT_RDWR); // only should fail if our socket
                                         // is already closed.
  
  exit(exitn);
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
