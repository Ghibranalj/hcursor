#ifndef HCURSOR_H_
#define HCURSOR_H_
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <sys/un.h>
#include <stdbool.h>
// hey
#define ACK_MSG "ACK"
#define KILL_MSG "KILL"


#define SOCK_BACKLOG 5

#define BUF_SIZE 32

typedef struct sockaddr_un sockaddr_t;

void hide(Display *display, Window root);
int parse_args(int argc, char **argv);
int hide_cursor();
int show_cursor();
int open_socket(char *path, sockaddr_t *addr);
bool is_cursor_hidden();
void print_status();
#endif // HCURSOR_H_
