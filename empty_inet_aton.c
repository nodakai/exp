#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
    struct in_addr addr;
    int rc = inet_aton("", &addr);
    in_port_t a = addr.s_addr;
    printf("rc==%s; 0x%08x, %d.%d.%d.%d\n", rc ? "Success" : "Failure", a, a&0xff, (a>>8)&0xff, (a>>16)&0xff, (a>>24)&0xff);
    return 0;

}
