#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <linux/if.h>
#include <net/if.h>
#include <net/ethernet.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN 1024
char errbuf[BUFLEN];

int main(int argc, char *argv[])
{
    int sock;
    const char *ifname;
    struct ifreq ifr;

    if (argc > 1)
        ifname = argv[1];
    else
        ifname = "eth0";

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr)) {
        snprintf(errbuf, sizeof errbuf, "ioctl(SIOCGIFHWADDR) on %s", ifname);
        perror(errbuf);
        exit(1);
    }

    unsigned char *sa_data = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n", sa_data[0], sa_data[1], sa_data[2], sa_data[3], sa_data[4], sa_data[5]);

    return 0;
}
