#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

int main(void)
{
    DIR *dp = NULL;
    struct dirent *dir = NULL;
    int fd = 0, cnt = 0;
    struct stat st;
    time_t oldest = -1;
#define BUFLEN 1024
    char buf[BUFLEN];
    struct tm *oldest_tm;

    if ((dp = opendir(".")) == NULL) {
        perror("opendir");
        exit(1);
    }
    errno = 0;
    while ((dir = readdir(dp)) != NULL) {
        if (errno) {
            perror("readdir");
            exit(1);
        }
        /*
        if ((fd = open(dir->d_name, O_RDONLY)) < 0) {
            perror("open");
            exit(1);
        }
        */
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;

        if (stat(dir->d_name, &st) < 0) {
            perror("stat");
            exit(1);
        }
        oldest = ((st.st_mtime < oldest || oldest == -1) ? st.st_mtime : oldest);
        ++cnt;
    }
    printf("Among %d files, oldest = %s\n", cnt, ctime(&oldest));
    if (closedir(dp) != 0) {
        perror("closedir");
        exit(1);
    }

    return 0;
}
