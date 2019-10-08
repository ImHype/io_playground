#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef __APPLE__
#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "io-toolkit.h"
#include "hash_map.h"

typedef int (*handle_t)(void *);

#define FD_NUM 1024

typedef struct loop_t {
    hashmap_t * hashmap;
    struct timeval tv;
    struct pollfd ** fds;
    int fd_size;
    int fd_crt;
    int backendfd;
} loop_t;

typedef struct watcher_t {
    int fd;
    handle_t work;
    handle_t cb;
    void * result;
    QUEUE queue;
    loop_t * loop;
    #ifndef __APPLE__
    struct epoll_event * ev;
    #endif
} watcher_t;

int epoll_remove_fd_watcher(watcher_t * watcher) ;
int epoll_init_loop(loop_t * loop);
int epoll_add_fd_watcher(loop_t * loop, int fd, handle_t work, handle_t cb);
int epoll_start_loop(loop_t * loop);