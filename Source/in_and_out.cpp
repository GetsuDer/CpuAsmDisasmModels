#include <cassert>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

char *mmap_file(char *file_in, int *file_size) 
{
    assert(file_in);
    struct stat file_stat;
    errno = 0;
    if (stat(file_in, &file_stat)) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return NULL;
    }
    ssize_t file_in_size = file_stat.st_size;
    *file_size = file_in_size;
    if (file_in_size <= 0) {
        fprintf(stderr, "Error: file %s has no data\n", file_in);
        return NULL;
    }
    // open file with source commands
    int fd_in = open(file_in, 0);
    if (fd_in < 0) {
        fprintf(stderr, "Error: Can`t open file %s\n", file_in);
        return NULL;
    }

    // mmap file
    char *commands = (char *)mmap(NULL, file_in_size, PROT_READ, MAP_SHARED, fd_in, 0);
    if (!commands) {
        fprintf(stderr, "Error: Can`t mmap file %s\n", file_in);
        return NULL;
    }
    if (close(fd_in)) {
        fprintf(stderr, "Error: Can`t close file descriptor %d\n", fd_in);
        return NULL;
    }
    return commands;
}
