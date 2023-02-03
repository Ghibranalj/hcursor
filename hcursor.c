/* https://unix.stackexchange.com/a/726059/119298 */
/* compile with -lX11 -lXfixes */
/* https://www.x.org/releases/current/doc/fixesproto/fixesproto.txt */
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdbool.h>

#include "hcursor.h"

char *socket_file = "/tmp/hcursor.sock";
char *usage = "Usage: hcursor [options]\n"
              "Options:\n"
              "  -h, --help           Show this help message\n"
              "  -v, --version        Show version information\n"
              "  -x, --hide           Hide cursor\n"
              "  -y, --show           Show cursor\n"
              "  -s, --status         Print status [shown/hidden]\n"
              "  -t, --toggle         Toggle cursor\n";

// when --hide or -x create a process that will hide the cursor
// this process creates and listes to a socket for a kill signal
// when --show or -s sends a kill signal to the socket
// and the process will die and the cursor will be shown
int main(int argc, char **argv) {
  int val = parse_args(argc, argv);
  switch (val) {
  case 0:
    return 0;
  case 1:
    return hide_cursor();
  case 2:
    return show_cursor();
  case 3:
    print_status();
    break;
  default:
    return 1;
  }
}

void print_status(){
  if (is_cursor_hidden()) {
    printf("hidden\n");
    return;
  }
  printf("shown\n");
}

bool is_cursor_hidden(){
 return access(socket_file, F_OK) != -1;
}

int hide_cursor() {
  // check if socket exists if it does return 1
  //
  //
  if (access(socket_file, F_OK) != -1) {
    fprintf(stderr, "hcursor: cursor is already hidden\n");
    return 1;
  }

  Display *display = XOpenDisplay(NULL);
  if (display == 0) {
    fprintf(stderr, "Cannot open display\n");
    return 1;
  }

  int screen = DefaultScreen(display);
  Window root = RootWindow(display, screen);
  XFixesHideCursor(display, root);
  XSync(display, True);

  // create fork and daemonize
  int pid = fork();
  if (pid != 0) {
    return 0;
  }
  setsid();

  // delete previous file and create new socket
  unlink(socket_file);
  sockaddr_t addr;
  int sfd = open_socket(socket_file, &addr);

  if (sfd == -1) {
    return 1;
  }

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    fprintf(stderr, "cannot bind socket\n");
    return 1;
  }

  if (listen(sfd, SOCK_BACKLOG) == -1) {
    fprintf(stderr, "cannot listen on socket\n");
    return 1;
  }

  char buf[BUF_SIZE];

  while (1) {
    int cfd = accept(sfd, NULL, NULL);
    if (cfd == -1) {
      fprintf(stderr, "cannot accept on socket\n");
      return 1;
    }
    if (read(cfd, buf, BUF_SIZE) == -1) {
      fprintf(stderr, "cannot read from socket\n");
      return 1;
    }
    close(cfd);
    if (strcmp(buf, KILL_MSG) == 0) {
      break;
    }
  }

  close(sfd);
  unlink(socket_file);

  XFixesShowCursor(display, root);
  XSync(display, True);
  return 0;
}

int show_cursor() {
  sockaddr_t addr;
  int sfd = open_socket(socket_file, &addr);

  if (sfd == -1) {
    fprintf(stderr, "Cannot open socket\n");
    return 1;
  }

  if (connect(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    fprintf(stderr, "cannot connect to socket\n");
    return 1;
  }

  char buf[BUF_SIZE];

  if (write(sfd, KILL_MSG, BUF_SIZE) == -1) {
    fprintf(stderr, "cannot write to socket\n");
    return 1;
  }

  close(sfd);
  return 0;
}

// return 0 to do nothing and exit 0
// return 1 to hide cursor
// return 2 to show cursor
// return 3 to show status
// other return values are errors
int parse_args(int argc, char **argv) {
  if (argc != 2) {
    printf("%s", usage);
    return 255;
  }
  if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
    printf("%s", usage);
    return 0;
  }

  if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
    // VERSION defined in Makefile
    printf("hcursor version %s\n", VERSION);
    return 0;
  }

  if (strcmp(argv[1], "-x") == 0 || strcmp(argv[1], "--hide") == 0) {
    return 1;
  }
  if (strcmp(argv[1], "-y") == 0 || strcmp(argv[1], "--show") == 0) {
    return 2;
  }
  if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--status") == 0) {
    return 3;
  }
  // -t or --toggle
  if (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "--toggle") == 0) {
    if (is_cursor_hidden()) {
      return 2;
    }
    return 1;
  }

  printf("%s", usage);
  return 255;
}

int open_socket(char *path, sockaddr_t *addr) {
  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sfd == -1) {
    fprintf(stderr, "socket error\n");
    return -1;
  }
  memset(addr, 0, sizeof(sockaddr_t));
  addr->sun_family = AF_UNIX;
  strncpy(addr->sun_path, path, sizeof(addr->sun_path) - 1);
  return sfd;
}
