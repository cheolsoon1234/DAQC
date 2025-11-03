#ifndef __SYSFS_HANDLER_H__
#define __SYSFS_HANDLER_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

int32_t write_sysfs(const char *path, const char *value);

#endif // __SYSFS_HANDLER_H__