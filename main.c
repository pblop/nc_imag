#include "common.h"
#include "termio.h"
#include "util.h"
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/ioctl.h>

#define INPUT_BUFSIZE 1000*1000

struct {
  int sockfd;
  int connfd;
  int wait_time;
} globals = {-1, -1, 0};

// Parent signal handlers
void parent_sigint_handler(int signum);
void setup_parent_handler(void);
void cleanup_exit(int exitn);

// Children signal handlers
void setup_child_handler(void);
void child_sigusr1_handler(int signum);
int connection_logic(int connfd, struct sockaddr_in6* client_addr, socklen_t* client_addr_size);

int save_file(char* foldername, unsigned char* data, int size);

int main(int argc, char* argv[])/*{{{*/
{
  int port, connfd;
  struct sockaddr_in6 addr;
  struct sockaddr_in6 client_addr;
  socklen_t client_addr_size;
  char client_addr_str[INET6_ADDRSTRLEN];

  if (argc == 0 || argc > 3)
  {
    fprintf(stderr, "Usage: ncimag <port> [wait time (default 0) ms]\n");
    return 1;
  }
  if (argc < 2){
    fprintf(stderr, "Usage: %s <port> [wait time (default 0) ms]\n", argv[0]);
    return 1;
  }

  setup_parent_handler();

  // TODO: Check if argv[1] really is a number.
  port = atoi(argv[1]);
  if (argc == 3)
    globals.wait_time = atoi(argv[2]);

  printf("Config: port=%d, wait_time=%dms\n", port, globals.wait_time);

  // Create the socket
  if ((globals.sockfd = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
  {
    perror("create socket");
    cleanup_exit(1);
  }
  
  if (setsockopt(globals.sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
  {
    perror("setsockopt");
    cleanup_exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = in6addr_any;
  addr.sin6_port = htons(port);

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
    client_addr_size = sizeof(client_addr);
    connfd = accept(globals.sockfd, (struct sockaddr*) &client_addr, &client_addr_size);
    if (connfd == -1)
    {
      perror("accept");
      cleanup_exit(1);
    }
    getpeername(connfd, (struct sockaddr *)&client_addr, &client_addr_size);
    if(inet_ntop(AF_INET6, &client_addr.sin6_addr, client_addr_str, sizeof(client_addr_str)))
      printf("[%s:%d] Connected!\n", client_addr_str, ntohs(client_addr.sin6_port));
    else
      printf("[unknown] Connected!\n");
    
    // The stuff needed to print the image (prompting the terminal for the
    // current screen size and stuff like that), will probably take some time.
    // I'm making a new thread to handle every new incomming connection.
    switch (fork())
    {
      case -1:
        perror("fork");
        cleanup_exit(1);
      case 0:
        exit(connection_logic(connfd, &client_addr, &client_addr_size));
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
int connection_logic(int connfd, struct sockaddr_in6* client_addr, socklen_t* client_addr_size)/*{{{*/
{
  char client_addr_str[INET6_ADDRSTRLEN];

  unsigned char buf[INPUT_BUFSIZE+1];
  int bytes_read, err, prev_size = 0, cur_size = 0;
  img_type_t img_type;
  image_t img;

  globals.connfd = connfd; // Save the connection file descriptor for the signal handler.
  
  // We must ignore sigints because they will be sent to us whenever our parent
  // process is being killed via ^C.
  // Our parent process will then SIGUSR1 us.
  setup_child_handler();

  // Get ip address of client
  getpeername(connfd, (struct sockaddr *)client_addr, client_addr_size);
  if(!inet_ntop(AF_INET6, &client_addr->sin6_addr, client_addr_str, sizeof(client_addr_str)))
    strcpy(client_addr_str, "unknown");

  for (EVER)
  {
    msleep(globals.wait_time);
    ioctl(connfd, FIONREAD, &cur_size);

    // While the client is still sending data, we just wait.
    if (prev_size == cur_size || cur_size > INPUT_BUFSIZE)
      break;
    prev_size = cur_size;
  }

  // Read the image from the socket
  bytes_read = read(connfd, buf, INPUT_BUFSIZE+1);
  switch (bytes_read)
  {
    case -1:
      perror("read");
      close(connfd);
      return 1;
    case 0:
      fprintf(stderr, "[%s:%d] Client closed the connection.\n", client_addr_str, ntohs(client_addr->sin6_port));
      close(connfd);
      return 1;
    case INPUT_BUFSIZE+1:
      swrite(connfd, "Input too big.\n");
      close(connfd);
      return 1;
    default:
      break; // If the number of bytes is normal, continue.
  }

  fprintf(stderr, "[%s:%d] Raw image size: %d.\n", client_addr_str, ntohs(client_addr->sin6_port), bytes_read);
  // Decode the image
  img_type = guess_image_type(buf, bytes_read);
  if (img_type == IMGT_UNKNOWN)
  {
    fprintf(stderr, "[%s:%d] Uploaded an unknown image.\n", client_addr_str, ntohs(client_addr->sin6_port));
    swrite(connfd, "Sorry, we don't support that image type at the moment\n");
    close(connfd);
    return 0;
  }

  if ((err = decode_image(&img, buf, bytes_read, img_type)) != 0)
  {
    fprintf(stderr, "[%s:%d] decode_image(&img, buf, bytes_read=%d, img_type=%d)=%d\n", client_addr_str, ntohs(client_addr->sin6_port), bytes_read, img_type, err);
    swrite(connfd, "Sorry, we had an error while decoding that image\n");
    close(connfd);
    
    err = save_file("errors/", buf, bytes_read);
    if (err >= 0)
      fprintf(stderr, "[%s:%d] error producing file saved as errors/%d.png\n", client_addr_str, ntohs(client_addr->sin6_port), err);
    else
      fprintf(stderr, "[%s:%d] error producing file could not be saved\n", client_addr_str, ntohs(client_addr->sin6_port));
    return 1;
  }

  print_image(connfd, &img);

  close(connfd);
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

int save_file(char* foldername, unsigned char* data, int size)/*{{{*/
{
  DIR *d;
  FILE *f;
  char filename[1024]; // 1024 is the max length of a filename on macos.
                       // (or at least it's the max size a dir->d_name can be.)
  char *filepath;
  int max_used_number = 0;

  struct dirent *dir;
  d = opendir(foldername);
  if (d == NULL)
    return -1;

  while ((dir = readdir(d)) != NULL) {
    sscanf(dir->d_name, "%s.%*s", filename);

    // TODO: Check if filename is a number.
    if (atoi(filename) > max_used_number)
      max_used_number = atoi(filename);
  }
  closedir(d);

  filepath = malloc(strlen(foldername) + strlen(filename) + 1);
  sprintf(filepath, "%s%d.png", foldername, max_used_number+1);
  
  f = fopen(filepath, "wb");
  if (f == NULL)
    return -2;

  fwrite(data, size, 1, f);
  free(filepath);
  return max_used_number+1;
}/*}}}*/

