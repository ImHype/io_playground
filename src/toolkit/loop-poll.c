#include "loop-poll.h"
#include "poll.h"

watcher_t * poll_init_watcher(int fd) {
    watcher_t * watcher = malloc(sizeof(watcher_t));
    watcher->fd = fd;
    return watcher;
}

int poll_add_fd_watcher(loop_t * loop, int fd, handle_t work, handle_t cb) {
    wq_t * wq = loop->wq;

    watcher_t * w = poll_init_watcher(fd);

    w->result = NULL;
    w->work = work;
    w->cb = cb;
    w->loop = loop;
    w->fd_idx = -1;

    QUEUE_INSERT_TAIL(&wq->queue, &w->queue);

    // FD_SET(fd, loop->fds);

    return 0;
}

int poll_remove_fd_watcher(watcher_t * watcher) {
    int fd = watcher->fd;
    loop_t * loop = watcher->loop;
    QUEUE_REMOVE(&watcher->queue);
    free(watcher);
    // FD_CLR(fd, loop->fds);
    return 0;
}

int init_wq(wq_t * wq) {
    QUEUE_INIT(&wq->queue);
    return 0;
}


struct pollfd * poll_init_fd_set(loop_t * loop, int size) {
    wq_t * wq = loop->wq;

    QUEUE * q;

    struct pollfd * fds = malloc(size * sizeof(struct pollfd));

    int i = 0;
    QUEUE_FOREACH(q, &wq->queue) {
        watcher_t * w = QUEUE_DATA(q, watcher_t, queue);

        int fd = w->fd;
        struct pollfd * h = fds + i;
        h->fd = fd;
        h->events = POLLIN;
        w->fd_idx = i;

        i++;
    }

    return fds;
}

int poll_init_loop(loop_t * loop) {
    loop->wq = malloc(sizeof(wq_t));
    
    init_wq(loop->wq);

    loop->tv.tv_sec = 5;
    loop->tv.tv_usec = 0;

    return 0;
}

int poll_start_loop(loop_t * loop) {
    while (1)
    {
        QUEUE * q;

        wq_t * wq = loop->wq;

        int maxfd = -1;
        int has_fd = 0;

        int count = 0;

        QUEUE_FOREACH(q, &wq->queue) {
            watcher_t * watcher = QUEUE_DATA(q, watcher_t, queue);
            int fd = watcher->fd;
            has_fd = 1;
            count++;
            
            if (fd > maxfd)
                maxfd = fd;
        }

        if (!has_fd) {
            return 0;
        }

        // fd_set tmpset = *fds;
        struct pollfd * fds = poll_init_fd_set(loop, count);

        int r = poll(fds, count, 5000);

        if (r < 0) do {
                perror("select()");
                break;
            } while(0);
        else if (r) {
            QUEUE * q;

            QUEUE_FOREACH(q, &loop->wq->queue) {
                watcher_t * watcher = QUEUE_DATA(q, watcher_t, queue);
                int fd = watcher->fd;

                if (watcher->fd_idx == -1) {
                    continue;
                }

                if ((fds + watcher->fd_idx)->revents & POLLIN) {
                    watcher->work(watcher);
                }
            }
        }
        else {
            printf("%d No data within five seconds.\n", r);
        }
    }

    free(loop->fds);
    free(loop);
}
