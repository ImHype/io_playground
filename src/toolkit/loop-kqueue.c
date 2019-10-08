#include "loop-epoll.h"
#include "hash_map.h"
#ifdef __APPLE__
#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif
#include <stdio.h>
#include <stdlib.h>


int epoll_add_fd_watcher(loop_t * loop, int fd, handle_t work, handle_t cb) {
    watcher_t * w = malloc(sizeof(watcher_t));

    w->fd = fd;
    w->result = NULL;
    w->work = work;
    w->cb = cb;
    w->loop = loop;

    loop->hashmap->put(loop->hashmap, fd, w);

    #ifndef __APPLE__
    struct epoll_event *ev = malloc(sizeof(struct epoll_event));
    ev->data.fd = fd;
    ev->events = EPOLLIN;
    if (epoll_ctl(loop->backendfd, EPOLL_CTL_ADD, fd, ev) < 0) {
      perror("epoll_ctl");
    }
    w->ev = ev;
    #endif

    return 0;
}

int epoll_remove_fd_watcher(watcher_t * watcher) {
    int fd = watcher->fd;
    loop_t * loop = watcher->loop;
    loop->hashmap->del(loop->hashmap, fd);

    #ifndef __APPLE__
    free(watcher->ev);
    if (epoll_ctl(loop->backendfd, EPOLL_CTL_DEL, fd, NULL) < 0) {
      perror("epoll_ctl");
    }
    #endif

    free(watcher);

    return 0;
}


int epoll_init_loop(loop_t * loop) {
    loop->hashmap = malloc(sizeof(hashmap_t));

    hash_map_init(loop->hashmap);

    loop->tv.tv_sec = 5;
    loop->tv.tv_usec = 0;

    #ifdef __APPLE__
    loop->backendfd = kqueue();

    #else
    loop->backendfd = epoll_create(1024);
    #endif

    return 0;
}


int epoll_start_loop_linux(loop_t * loop) {
    return 0;
}

#ifdef __APPLE__
int epoll_start_loop(loop_t * loop) {
    while (1)
    {
        int maxfd = -1;

        struct kevent events[1024];
        hashmap_t * map = loop->hashmap;
        int nevents = map->size;

        int * keys = map->keys(map);

        for (int i = 0; i < nevents; i++)
        {
            int fd = keys[i];
            
            EV_SET(events + i, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
        }

        free(keys);

        if (nevents == 0) {
            return 0;
        }

        struct timespec spec;

        int timeout = 5000;

        spec.tv_sec = timeout / 1000;
        spec.tv_nsec = (timeout % 1000) * 1000000;


        int kqueue_size = kevent(loop->backendfd, events, nevents, events, 1024, timeout == -1 ? NULL : &spec);

        if (kqueue_size < 0) do {
                perror("kevent");
                break;
            } while(0);
        else if (kqueue_size) {
            struct kevent * ev;
            for (int i = 0; i < kqueue_size; i++)
            {
                ev = events + i;

                if( ev->flags & EV_ERROR ) {
                    perror("Event error");
                }

                int fd = ev->ident;

                watcher_t * watcher = (watcher_t *) map->get(map, fd);

                watcher->work(watcher);
            }
        }
        else {
            printf("%d No data within five seconds.\n", kqueue_size);
        }
    }

    free(loop->fds);
    free(loop);
}
#else
int epoll_start_loop(loop_t * loop) {
    while (1)
    {
        int maxfd = -1;
        hashmap_t * map = loop->hashmap;
        int nevents = map->size;

        if (nevents == 0) {
            return 0;
        }

        struct timespec spec;

        int timeout = 5000;

        spec.tv_sec = timeout / 1000;
        spec.tv_nsec = (timeout % 1000) * 1000000;

        struct epoll_event evts[1024];
        int kqueue_size = epoll_wait(loop->backendfd, evts, nevents, timeout == -1 ? NULL : &spec);
        if (kqueue_size < 0) do {
                perror("kevent");
                break;
            } while(0);
        else if (kqueue_size) {
            for (int i = 0; i < kqueue_size; i++)
            {
                struct epoll_event * ev = evts + i;

                int fd = ev->data.fd;

                if (ev->events & EPOLLIN) {
                    watcher_t * watcher = (watcher_t *) map->get(map, fd);

                    watcher->work(watcher);
                }
            }
        }
        else {
            printf("%d No data within five seconds.\n", kqueue_size);
        }
    }

    free(loop->fds);
    free(loop);
}
#endif