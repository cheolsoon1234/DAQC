#ifndef __LOGIN_H__
#define __LOGIN_H__

#include <utmp.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int32_t Check_Login(void);

#endif // __LOGIN_H__