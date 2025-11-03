#include "sysfs_handler.h"

int32_t write_sysfs(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror(path);
        exit(1);
        return 0;
    }
    write(fd, value, strlen(value));
    close(fd);
    return 1;
}