#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sys/types.h>
#include <regex.h>
#include "io-toolkit.h"


int socket_fd;
char res[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8\r\nConnection: keep-alive\r\n\r\n<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><title>Document</title></head><body><p>this is http response</p></body></html>";

void graceful() {
    close(socket_fd);
    exit(0);
}

int cflags = 0000;
regex_t reg;

int main(int argc, char const *argv[])
{
    regcomp(&reg, "\r\n$", REG_EXTENDED);

    struct sockaddr_in address;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    socklen_t seraddrlen;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0); 

    printf("opened socket_fd: %d\n", socket_fd);

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

    if (listen(socket_fd, 5) < 0) {
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
        }
        
        cliaddrlen = sizeof(cliaddr);

        char buf[1000];
        memset(buf, 0, sizeof(buf));
        while (read(client_fd, buf, sizeof(buf)) > 0)
        {
            printf("regexec\n");
            regmatch_t pmatch;
            int status;
            
            status = regexec(&reg, buf, 1, &pmatch, 0000);
            printf("%s\n", buf);

            if (0 == status) {
                break;
            }
        }

        printf("req: %s\n", buf);

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

    return 0;
}
