#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char const *argv[])
{
    char buffer[1024];

    char * fifo_name = "a.fifo";

    int res = mkfifo(fifo_name, 0777);
    int fd = open("a.fifo", O_RDONLY | O_CREAT, 0666);
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