#include "lab3.h"

void freeInfo(Info *info) {
    free(info->from);
    free(info->where);
    free(info);
}

void *copyingFile(void *arg) {
    Info *info = (Info *)arg;

    char buffer[BUFFER_SIZE];
    ssize_t countRead, countWrite;

    errno = 0;
    int fileFrom = open(info->from, O_RDONLY);
    
    while (errno == EMFILE) {
        sleep(3);
        errno = 0;
        fileFrom = open(info->from, O_RDONLY);
    }

    if (fileFrom == -1) {
        printf("Error opening 'from' file: %s\n", info->from);
        freeInfo(info);
        pthread_exit(NULL);
    }

    int fileWhere = open(info->where, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fileWhere == -1) {
        printf("Error opening 'where' file: %s\n", info->where);
        close(fileFrom);
        freeInfo(info);
        pthread_exit(NULL);
    }

    while ((countRead = read(fileFrom, buffer, BUFFER_SIZE)) > 0) {
        countWrite = write(fileWhere, buffer, (size_t)countRead);
        if (countRead != countWrite) {
            printf("Error writing to 'where' file: %s\n", info->where);
            close(fileFrom);
            close(fileWhere);
            freeInfo(info);
            pthread_exit(NULL);
        }
    }

    close(fileFrom);
    close(fileWhere);
    freeInfo(info);
    pthread_exit(NULL);
}

void *copyingDirectory(void *arg) {
    Info *info = (Info *)arg;

    if (mkdir(info->where, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        printf("Error creating 'where' directory: %s\n", info->where);
        freeInfo(info);
        pthread_exit(NULL);
    }

    errno = 0;
    DIR *dir = opendir(info->from);
    while (errno == EMFILE) {
        sleep(3);
        errno = 0;
        dir = opendir(info->from);
    }
    if (!dir) {
        printf("Error opening 'from' directory: %s\n", info->from);
        freeInfo(info);
        pthread_exit(NULL);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char entryFromPath[PATH_MAX];
        snprintf(entryFromPath, PATH_MAX, "%s/%s", info->from, entry->d_name);

        char entryWherePath[PATH_MAX];
        snprintf(entryWherePath, PATH_MAX, "%s/%s", info->where, entry->d_name);

        struct stat entryFromStat;
        if (stat(entryFromPath, &entryFromStat) == -1) {
            printf("Error getting file status: %s\n", entryFromPath);
            continue;
        }

        if (S_ISDIR(entryFromStat.st_mode)) {
            pthread_t thread;
            Info *dirInfo = malloc(sizeof(Info));
            dirInfo->from = strdup(entryFromPath);
            dirInfo->where = strdup(entryWherePath);
            int err = pthread_create(&thread, NULL, copyingDirectory, dirInfo);
            if (err) {
                printf("Error creating thread for copying %s: %s\n", dirInfo->from, strerror(err));
                freeInfo(dirInfo);
            }
        } else if (S_ISREG(entryFromStat.st_mode)) {
            pthread_t thread;
            Info *fileInfo = malloc(sizeof(Info));
            fileInfo->from = strdup(entryFromPath);
            fileInfo->where = strdup(entryWherePath);
            int err = pthread_create(&thread, NULL, copyingFile, fileInfo);
            if (err) {
                printf("Error creating thread for copying %s: %s\n", fileInfo->from, strerror(err));
                freeInfo(fileInfo);
            }
        }
    }

    closedir(dir);
    freeInfo(info);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Error: Wrong number of arguments\n");
        return -1;
    }

    Info *startInfo = malloc(sizeof(Info));
    startInfo->from = strdup(argv[1]);
    startInfo->where = strdup(argv[2]);

    pthread_t thread;
    int err = pthread_create(&thread, NULL, copyingDirectory, startInfo);
    if (err) {
		printf("Error creating thread for copying %s: %s\n", startInfo->from, strerror(err));
        freeInfo(startInfo);
	}

    pthread_exit(NULL);
}