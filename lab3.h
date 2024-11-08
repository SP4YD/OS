#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

typedef struct {
    char *from;
    char *where;
} Info;

void freeInfo(Info *info);
void *copyingFile(void *arg);
void *copyingDirectory(void *arg);