#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "io-toolkit.h"

typedef int (*handle_t)(void *);

typedef struct wq_t {
    QUEUE queue;
} wq_t;


typedef struct loop_t {
    wq_t * wq;
    struct timeval tv;
    fd_set * fds;
} loop_t;

typedef struct watcher_t {
    int fd;
    handle_t work;
    handle_t cb;
    void * result;
    QUEUE queue;
    loop_t * loop;
} watcher_t;

int select_remove_fd_watcher(watcher_t * watcher) ;
int select_init_loop(loop_t * loop);
int select_add_fd_watcher(loop_t * loop, int fd, handle_t work, handle_t cb);
int select_start_loop(loop_t * loop);