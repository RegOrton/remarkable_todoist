/*
 * Minimal inotifywait replacement for reMarkable launcher
 * Watches a single file for access/open events
 * Usage: inotifywait -q -e open,access <file>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    const char *path = NULL;
    uint32_t mask = 0;
    int quiet = 0;

    /* Parse args compatible with: inotifywait -q -e open,access <file> */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-q") == 0) {
            quiet = 1;
        } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            i++;
            char *events = strdup(argv[i]);
            char *tok = strtok(events, ",");
            while (tok) {
                if (strcmp(tok, "open") == 0)    mask |= IN_OPEN;
                if (strcmp(tok, "access") == 0)  mask |= IN_ACCESS;
                if (strcmp(tok, "modify") == 0)  mask |= IN_MODIFY;
                if (strcmp(tok, "close") == 0)   mask |= IN_CLOSE;
                if (strcmp(tok, "create") == 0)  mask |= IN_CREATE;
                if (strcmp(tok, "delete") == 0)  mask |= IN_DELETE;
                if (strcmp(tok, "attrib") == 0)  mask |= IN_ATTRIB;
                tok = strtok(NULL, ",");
            }
            free(events);
        } else if (argv[i][0] != '-') {
            path = argv[i];
        }
    }

    if (!path || !mask) {
        fprintf(stderr, "Usage: inotifywait [-q] -e <events> <file>\n");
        return 1;
    }

    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return 1;
    }

    int wd = inotify_add_watch(fd, path, mask);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        return 1;
    }

    /* Wait for one event */
    char buf[BUF_SIZE];
    ssize_t len = read(fd, buf, sizeof(buf));
    if (len > 0 && !quiet) {
        printf("%s\n", path);
    }

    close(fd);
    return 0;
}
