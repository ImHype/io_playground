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
#include "io-toolkit.h"

char res[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8\r\nConnection: keep-alive\r\n\r\n<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><title>Document</title></head><body><p>this is http response</p></body></html>";

QUEUE queue;

#define DEFAULT_THREAD_SIZE 200

pthread_t thread_pool[DEFAULT_THREAD_SIZE];


int socket_fd;

void graceful() {
    close(socket_fd);
    exit(0);
}

int cflags = 0x0000;
regex_t reg;

sem_t * sem;
pthread_mutex_t mutex;

void handle(client_t * client) {
    int client_fd = client->fd;
    char buf[100];
    char request[1024];
    memset(buf, 0, sizeof(buf));
    memset(request, 0, sizeof(buf));

    int nread;

    while ((nread = read(client_fd, buf, sizeof(buf))) > 0)
    {
        regmatch_t pmatch;
        
        strcat(request, buf);
        memset(buf, 0, sizeof(buf));

        if (0 == regexec(&reg, request, 1, &pmatch, 0000)) {
            // printf("%s\n", request);
            break;
        }
    }

    if (write(client_fd, res, strlen(res)) < 0) {
        perror("write");
    }

    if (shutdown(client_fd, SHUT_RDWR) < 0) {
        perror("shutdown");
    };

    int size = read(client_fd, buf, sizeof(buf));

    if (size < 0) {
        perror("read");
    }

    if (close(client_fd) < 0) {
        perror("close");
    }
}

void* thread_main() {
    while (1)
    {
        sem_wait(sem);

        pthread_mutex_lock(&mutex);
        QUEUE * node = QUEUE_HEAD(&queue);
        QUEUE_REMOVE(node);
        client_t * client = QUEUE_DATA(node, client_t, queue);
        pthread_mutex_unlock(&mutex);

        client->handle(client);

        pthread_mutex_lock(&mutex);
        free(client);
        pthread_mutex_unlock(&mutex);
    }
    
    return 0;
}

int init_threads() {
    for (int i = 0; i < DEFAULT_THREAD_SIZE; i++)
    {
        pthread_create(thread_pool + i, NULL, thread_main, NULL);
    }

    return 0;
}


int init_sem() {
    #ifdef __APPLE__
    sem_unlink("sem");
    sem = sem_open("sem", O_CREAT, 0777, 0);
    #else
    sem = malloc(sizeof(sem_t));
    sem_init(sem, 0, 0);
    #endif

    pthread_mutex_init(&mutex, 0);
    return 0;
}

int main(int argc, char const *argv[])
{
    QUEUE_INIT(&queue);
    init_sem();
    init_threads();

    regcomp(&reg, "\r\n$", REG_EXTENDED);

    struct sockaddr_in address;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
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

    while (1)
    {
        int client_fd, ret;
        client_fd = accept(socket_fd, (struct sockaddr*)&cliaddr, &cliaddrlen);

        if (client_fd < 0) {
            perror("accept");
            continue;
        } else {
            client_t * client = (client_t *) malloc(sizeof(client_t));
            client->fd = client_fd;
            client->req_parsed = 0;
            client->handle = handle;

            memset(client->buf, 0, sizeof(client->buf));

            pthread_mutex_lock(&mutex);
            QUEUE_INSERT_TAIL(&queue, &client->queue);
            pthread_mutex_unlock(&mutex);

            sem_post(sem);
        }
    }

    return 0;
}
