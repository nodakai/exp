#include <stdio.h>
#include <sys/socket.h>

int main(void)
{
    int val;
    socklen_t len;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    getsockopt(s, SOL_SOCKET, SO_RCVBUF, &val, &len);
    printf("getsockopt(SO_RCVBUF) => %d\n", val);
    getsockopt(s, SOL_SOCKET, SO_SNDBUF, &val, &len);
    printf("getsockopt(SO_SNDBUF) => %d\n", val);

    val = 30 * 1024 * 1024;
    printf("setsockopt(SO_RCVBUF, %d) => %d\n", val, setsockopt(s, SOL_SOCKET, SO_RCVBUF, &val, sizeof val));
    val = 30 * 1024 * 1024;
    printf("setsockopt(SO_SNDBUF, %d) => %d\n", val, setsockopt(s, SOL_SOCKET, SO_SNDBUF, &val, sizeof val));

    getsockopt(s, SOL_SOCKET, SO_RCVBUF, &val, &len);
    printf("getsockopt(SO_RCVBUF) => %d\n", val);
    getsockopt(s, SOL_SOCKET, SO_SNDBUF, &val, &len);
    printf("getsockopt(SO_SNDBUF) => %d\n", val);
    return 0;
}
