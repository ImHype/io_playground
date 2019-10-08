#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sys/types.h>
#include <regex.h>
#include <pthread.h>
#include <semaphore.h>
#include "loop-select.h"

char res[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8\r\nConnection: keep-alive\r\n\r\n<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><title>Document</title></head><body><p>this is http response</p></body></html>";

int socket_fd;

void graceful() {
    close(socket_fd);
    exit(0);
}
regex_t reg;

int send_socket(watcher_t * w) {
    int client_fd = w->fd;
    char buf[100];

    if (write(client_fd, res, strlen(res)) < 0) {
        perror("write");
    }

    if (shutdown(client_fd, SHUT_RDWR) < 0) {
        perror("shutdown");
    };

    return 0;
}


int read_socket(void * argv) {
    watcher_t * w = (watcher_t *) argv;
    int fd = w->fd;

    int SIZE = 100;
    char * buf = (char *) malloc(SIZE);
    memset(buf, 0, SIZE);

    int n = read(fd, buf, SIZE);

    if (n < 0) {
        perror("read");
    };

    if (n == 0) {
        free(buf);
        free(w->result);
        select_remove_fd_watcher(w);

        if (close(fd) < 0) {
            perror("close");
        }
    } else {
        int s_a = w->result ? strlen(w->result): 0;
        int s_b = buf ? strlen(buf): 0;
        int length = s_a + s_b;
        char * tmp = malloc(length);

        memset(tmp, 0, length);

        if (w->result) {
            strcat(tmp, w->result);
            free(w->result);
        }

        strcat(tmp, buf);
        free(buf);

        w->result = tmp;
        regmatch_t pmatch;

        // printf("result: %s\n", w->result);
        int status = regexec(&reg, w->result, 1, &pmatch, 0);

        if (0 == status) {
            w->cb(w);
        } else {
            printf("reg result %d\n", status);
            // printf("reg result: %d %s\n", s, w->result);
        }
    }

    return n;
}

int read_socket_cb(void * argv) {
    send_socket(argv);
    return 0;
}


int accept_socket(void * argv) {
    watcher_t * w = (watcher_t *) argv;

    struct sockaddr * cliaddr = malloc(sizeof(struct sockaddr_in));
    socklen_t * cliaddrlen = malloc(sizeof(socklen_t));

    int fd = accept(socket_fd, cliaddr, cliaddrlen);

    free(cliaddr);
    free(cliaddrlen);

    if (fd > 0) {
        select_add_fd_watcher(w->loop, fd, read_socket, read_socket_cb);
    } else {
        perror("accept");
    }

    return 1;
}

int accept_socket_cb(void * argv) {
    watcher_t * w = (watcher_t *) argv;

    return 0;
}

int main(int argc, char const *argv[])
{
    regcomp(&reg, "\r\n$", REG_EXTENDED);

    struct sockaddr_in address;
    socklen_t seraddrlen;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0); 

    int enable_reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("0.0.0.0");
    address.sin_port = htons(3000);
    
    if (bind(socket_fd, (struct sockaddr*)&address,  sizeof(address)) < 0 ) {
        perror("bind");
        exit(1);
    };

    if (listen(socket_fd, 20) < 0) {
        perror("listen");
        exit(1);
    }

    loop_t loop;

    select_init_loop(&loop);
    
    select_add_fd_watcher(&loop, socket_fd, accept_socket, accept_socket_cb);
    
    select_start_loop(&loop);

    return 0;
}
