#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// #include <sys/ioctl.h>
// #include <errno.h>
// nonblock_ioctl(fd, 1);

// int nonblock_ioctl(int fd, int set) {
//   int r;

//   do
//     r = ioctl(fd, FIONBIO, &set);
//   while (r == -1 && errno == EINTR);

//   return 0;
// }

int main() {
    //创建套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    //向服务器（特定的IP和端口）发起请求
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("61.135.169.125");  //具体的IP地址
    serv_addr.sin_port = htons(80);  //端口
    connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
   
    //读取服务器传回的数据
    char * req = "GET / HTTP/1.0\r\nHost: www.baidu.com\r\nAccept: */*\r\n\r\n";
    write(fd, req, strlen(req));

    int base = 1024;
    int factor = 2;

    char * res = malloc(factor * base);
    memset(res, 0 , factor * base);

    int total = 0;
    
    char buffer[base];
    int recieved_bytes = 0;
    
    while ((recieved_bytes = read(fd, buffer, sizeof(buffer))) > 0)
    {
        total = total + recieved_bytes;

        if (total >= base * factor) {
            factor++;
            char * tmp = malloc(factor * base);
            memset(tmp, 0 , factor * base);
            memcpy(tmp, res, strlen(res));
            res = tmp;
        }

        strcat(res, buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    if (recieved_bytes < 0) {
        perror("read");
        return -1;
    }

    printf("%s\n", res);

    free(res);
    //关闭套接字
    close(fd);
    return 0;
}