#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    char buffer[1024];
    int fd = open("index.html", O_RDONLY, 0666);
    int size;

    if (fd < 0) {
        perror("open");
        return 1;
    }

    while ((size = read(fd, buffer, sizeof(buffer))) > 0)
    {
        printf("%s\n", buffer);
    }

    if (size < 0) {
        perror("read");
    }

    close(fd);
    return 0;
}