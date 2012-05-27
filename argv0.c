#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/stat.h>

enum EnumStatus {
    SUCCESS,
    ERR_EMPTY_PATH,
    ERR_INSUF_BUF,
    ERR_NOT_FOUND_IN_PATH
};

int search_from_PATH(const char *argv0, char *buf, size_t buflen)
{
    char path[FILENAME_MAX];
    const char * const P = getenv("PATH");
    if (NULL == P) return ERR_EMPTY_PATH;

    strncpy(path, P, sizeof path);

    char *save = NULL, *tok;
    const char delim[] = ":";
    for (tok = strtok_r(path, delim, &save); tok; tok = strtok_r(NULL, delim, &save)) {
        char fullpath[FILENAME_MAX];
        strncpy(fullpath, tok, sizeof fullpath);
        const size_t toklen = strlen(tok);
        strncat(fullpath, "/", sizeof fullpath - toklen);
        strncat(fullpath, argv0, sizeof fullpath - toklen - 1);
        struct stat st;
        if (0 == stat(fullpath, &st) && S_IFREG & st.st_mode) {
            if (strlen(fullpath) + 1 <= buflen) {
                strncpy(buf, fullpath, buflen);
                return 0;
            } else {
                return ERR_INSUF_BUF;
            }
        }
    }
    return ERR_NOT_FOUND_IN_PATH;
}

int get_self(const char *argv0, char *buf, size_t buflen)
{
    char *found_slash = strrchr(argv0, '/');
    if (found_slash) {
        if ('/' == argv0[0]) {
            strncpy(buf, argv0, buflen);
            return SUCCESS;
        } else {
            getcwd(buf, buflen);
            strcat(buf, "/");
            strcat(buf, argv0);
            return SUCCESS;
        }
    } else {
        return search_from_PATH(argv0, buf, buflen);
    }
}

int main(int argc, char *argv[])
{
    if (argc == 0) abort();
    const char *argv0 = argv[0];

    char buf[FILENAME_MAX];
    int rc;
    if (SUCCESS == (rc = get_self(argv0, buf, sizeof buf))) {
        printf("I am \"%s\"\n", buf);
        return SUCCESS;
    } else {
        fprintf(stderr, "Failed to obtain the abs path to this executable. Err==%d\n", rc);
        return 100;
    }
}
