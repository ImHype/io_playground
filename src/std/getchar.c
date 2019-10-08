#include <unistd.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    char buf[100];
    read(0, buf, sizeof(buf));

    printf("%s\n", buf);
    return 0;
}
