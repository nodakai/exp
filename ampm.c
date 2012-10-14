#define _XOPEN_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#define BUFLEN 2048

int main(int argc, char *argv[])
{
    time_t nowTime_t;
    struct tm nowTm;
    char buf[BUFLEN];
    const char fmt[] = "%F %T: %l %I %p %P %s";

    if (1 == argc) {
        nowTime_t = time(NULL);
        localtime_r(&nowTime_t, &nowTm);
        strftime(buf, sizeof buf, fmt, &nowTm);
    } else {
        snprintf(buf, sizeof buf, "%s %s", argv[1], argv[2]);
        strptime(buf, "%F %T", &nowTm);
        strftime(buf, sizeof buf, fmt, &nowTm);
    }
    printf("[%02d:%02d:%02d] ", nowTm.tm_hour, nowTm.tm_min, nowTm.tm_sec);
    puts(buf);

    return 0;
}
