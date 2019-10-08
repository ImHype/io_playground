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
    struct pollfd ** fds;
    int fd_size;
    int fd_crt;
} loop_t;

typedef struct watcher_t {
    int fd;
    handle_t work;
    handle_t cb;
    void * result;
    QUEUE queue;
    loop_t * loop;

    int fd_idx;
} watcher_t;

int poll_remove_fd_watcher(watcher_t * watcher) ;
int poll_init_loop(loop_t * loop);
int poll_add_fd_watcher(loop_t * loop, int fd, handle_t work, handle_t cb);
int poll_start_loop(loop_t * loop);