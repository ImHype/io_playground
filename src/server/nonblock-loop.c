#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sys/types.h>
#include <regex.h>
#include <fcntl.h>
#include "io-toolkit.h"

char res[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8\r\nConnection: keep-alive\r\n\r\n<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><title>Document</title></head><body><p>this is http response</p></body></html>";

int cflags = 0000;
int socket_fd;
regex_t reg;
QUEUE clients;

void graceful() {
    close(socket_fd);
    exit(0);
}


int non_block(fd) {
    return fcntl(fd, F_SETFL, O_NONBLOCK);
}

int check_clients() {
    QUEUE * item;
    QUEUE_FOREACH(item, &clients) {
        client_t * client = QUEUE_DATA(item, client_t, queue);
        int fd = client->fd;
        int nread;
        char buf[100];

        memset(buf, 0, sizeof(buf));

        while ((nread = read(fd, buf, sizeof(buf))) > 0)
        {
            strcat(client->buf, buf);
            memset(buf, 0, sizeof(buf));
        }
        regmatch_t pmatch;

        if (client->req_parsed == 0) {
            int status = regexec(&reg, client->buf, 1, &pmatch, cflags);
            if (status == 0) {
                client->req_parsed = 1;
                client->handle(client);
            }
        }

        if (nread < -1) {
            perror("read");
        }

        if (nread < -1 || nread == 0) {
            if (close(fd) < 0) {
                perror("close");
            }

            QUEUE_REMOVE(item);
            free(client);
        }
    }
    return 0;
};

int init_reg(regex_t * reg) {
    regcomp(reg, "\r\n$", REG_EXTENDED);
    return 0;
}

int init_address(struct sockaddr_in * address) {
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = inet_addr("0.0.0.0");
    address->sin_port = htons(3000);

    return 0;
}

void handle(client_t * client) {
    int fd = client->fd;

    if (write(fd, res, strlen(res)) < 0) {
        perror("write");
    }

    if (shutdown(fd, SHUT_RDWR) < 0) {
        perror("shutdown");
    };

    char buf[1024];

    int nsize = read(fd, buf, sizeof(buf));

}

client_t * init_client(fd) {
    client_t * client = (client_t *) malloc (sizeof(client_t));
    client->fd = fd;
    memset(client->buf, 0, sizeof(client->buf));
    client->handle = handle;
    client->req_parsed = 0;

    return client;
}

int main(int argc, char const *argv[])
{
    signal(SIGHUP, graceful);
    signal(SIGINT, graceful);
    signal(SIGQUIT, graceful);

    struct sockaddr_in address;
    struct sockaddr_in cliaddr;
 
    socklen_t cliaddrlen;
    socklen_t seraddrlen;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    printf("opened socket_fd: %d\n", socket_fd);
 
    int enable_reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    init_reg(&reg);
    init_address(&address);

    if (bind(socket_fd, (struct sockaddr*)&address,  sizeof(address)) < 0 ) {
        perror("bind");
        exit(1);
    };

    if (listen(socket_fd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    non_block(socket_fd);

    QUEUE_INIT(&clients);

    while (1)
    {
        int client_fd = accept(socket_fd, (struct sockaddr*)&cliaddr, &cliaddrlen);

        if (client_fd > -1) {
            non_block(client_fd);
            client_t * client = init_client(client_fd);
            QUEUE_INSERT_TAIL(&clients, &client->queue);
        }

        usleep(10);

        check_clients();
    }

    return 0;
}