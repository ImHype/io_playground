#include "loop-select.h"

watcher_t * select_init_watcher(int fd) {
    watcher_t * watcher = malloc(sizeof(watcher_t));
    watcher->fd = fd;
    return watcher;
}

int select_add_fd_watcher(loop_t * loop, int fd, handle_t work, handle_t cb) {
    wq_t * wq = loop->wq;

    watcher_t * w = select_init_watcher(fd);

    w->result = NULL;
    w->work = work;
    w->cb = cb;
    w->loop = loop;

    QUEUE_INSERT_TAIL(&wq->queue, &w->queue);

    FD_SET(fd, loop->fds);

    return 0;
}

int select_remove_fd_watcher(watcher_t * watcher) {
    int fd = watcher->fd;
    loop_t * loop = watcher->loop;
    QUEUE_REMOVE(&watcher->queue);
    free(watcher);
    FD_CLR(fd, loop->fds);
    return 0;
}

int select_init_wq(wq_t * wq) {
    QUEUE_INIT(&wq->queue);
    return 0;
}


int select_init_fd_set(loop_t * loop) {
    wq_t * wq = loop->wq;

    fd_set * fds = malloc(sizeof(fd_set));
    FD_ZERO(fds);

    loop->fds = fds;

    return 0;
}

int select_init_loop(loop_t * loop) {
    loop->wq = malloc(sizeof(wq_t));
    
    select_init_wq(loop->wq);
    select_init_fd_set(loop);

    return 0;
}

int select_start_loop(loop_t * loop) {
    while (1)
    {

        fd_set * fds = loop->fds;

        QUEUE * q;

        wq_t * wq = loop->wq;


        int fd_size = 0;

        QUEUE_FOREACH(q, &wq->queue) {
            fd_size++;
        }
   
        if (fd_size == 0) {
            return 0;
        }

        printf("fd_size %d\n", fd_size);

        fd_set tmpset = *fds;

        struct timeval tv;
        tv.tv_sec = 5;

        int r = select(fd_size, &tmpset, NULL, NULL, &tv);

        if (r < 0) do {
                perror("select()");
                break;
            } while(0);
        else if (r) {
            printf("fd_size %d\n", r);
            QUEUE * q;
            QUEUE_FOREACH(q, &loop->wq->queue) {
                watcher_t * watcher = QUEUE_DATA(q, watcher_t, queue);
                int fd = watcher->fd;
                if (FD_ISSET(fd, &tmpset)) {
                    int n = watcher->work(watcher);
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
