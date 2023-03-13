#include "common.h"
#include "termio.h"
#include "util.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct {
  int sockfd;
  int connfd;
} globals = {-1, -1};

// Parent signal handlers
void parent_sigint_handler(int signum);
void setup_parent_handler(void);
void cleanup_exit(int exitn);

// Children signal handlers
void setup_child_handler(void);
void child_sigusr1_handler(int signum);
int connection_logic(int connfd);

int main(int argc, char* argv[])/*{{{*/
{
  int port, connfd;
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

  setup_parent_handler();

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

  if (listen(globals.sockfd, 100) == -1)
  {
    perror("listen");
    cleanup_exit(1);
  }
  printf("Listening on port %d\n", port);

  for (EVER)
  {
    connfd = accept(globals.sockfd, (struct sockaddr*) NULL, 0);
    if (connfd == -1)
    {
      perror("accept");
      cleanup_exit(1);
    }
    
    // The stuff needed to print the image (prompting the terminal for the
    // current screen size and stuff like that), will probably take some time.
    // I'm making a new thread to handle every new incomming connection.
    switch (fork())
    {
      case -1:
        perror("fork");
        cleanup_exit(1);
      case 0:
        exit(connection_logic(connfd));
      default:
        // We must close the file descriptor in the parent, because the child
        // has now duplicated it. If we don't close it, whenever the child calls
        // close on it, the connection will remain open, because there's still
        // a process (this parent process) with the file descriptor still open.
        close(connfd);
        break;
    }
  }

  return 0;
}/*}}}*/

// This is the main function for all incomming connections.
int connection_logic(int connfd)/*{{{*/
{
  globals.connfd = connfd; // Save the connection file descriptor for the signal handler.
  
  // We must ignore sigints because they will be sent to us whenever our parent
  // process is being killed via ^C.
  // Our parent process will then SIGUSR1 us.
  setup_child_handler();

  write(connfd, "Hello, world!", 13);

  if (close(connfd) == -1)
  {
    perror("close connection");
    return 1;
  }
  return 0;
}/*}}}*/

void parent_sigint_handler(int signum)/*{{{*/
{
  UNUSED(signum);
  cleanup_exit(0);
}/*}}}*/
void setup_parent_handler(void)/*{{{*/
{
  sigset_t empty_set;
  struct sigaction action;

  // Capture SIGINT and exit program safely then.
  sigemptyset(&empty_set);
  action.sa_handler = parent_sigint_handler;
  action.sa_mask = empty_set;
  action.sa_flags = SA_RESTART;
  sigaction(SIGINT, &action, NULL);

  // Ignore SIGUSR1, we will use this to kill our children.
  action.sa_handler = SIG_IGN;
  action.sa_mask = empty_set;
  action.sa_flags = SA_RESTART;
  sigaction(SIGUSR1, &action, NULL);
}/*}}}*/
void cleanup_exit(int exitn)/*{{{*/
{
  if (globals.sockfd != -1)
    shutdown(globals.sockfd, SHUT_RDWR); // only should fail if our socket
                                         // is already closed.

  kill(0, SIGUSR1); // Kill all our children.
  // TODO: Maybe wait for our children to actually die.
  
  exit(exitn);
}/*}}}*/

void setup_child_handler(void)/*{{{*/
{
  sigset_t empty_set;
  struct sigaction action;

  // Capture SIGINT and ignore it.
  sigemptyset(&empty_set);
  action.sa_handler = SIG_IGN;
  action.sa_mask = empty_set;
  action.sa_flags = SA_RESTART;
  sigaction(SIGINT, &action, NULL);

  // Cleanup on SIGUSR1.
  action.sa_handler = child_sigusr1_handler;
  action.sa_mask = empty_set;
  action.sa_flags = SA_RESTART;
  sigaction(SIGUSR1, &action, NULL);
}/*}}}*/
void child_sigusr1_handler(int signum)/*{{{*/
{
  UNUSED(signum);
  
  if (globals.connfd != -1)
    close(globals.connfd);

  exit(0);
}/*}}}*/

