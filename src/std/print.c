#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char * buf = "hello world";
    write(1, buf, strlen(buf));
    return 0;
}
