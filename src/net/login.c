#include "login.h"
#include <stdio.h>

int32_t Check_Login(void) {
    struct utmp *entry;

    /* 1. utmp 확인 */
    setutent();
    while ((entry = getutent()) != NULL) {
        if (entry->ut_type == USER_PROCESS) {
            endutent();
            return 1;
        }
    }
    endutent();

    /* 2. 환경 변수 확인 */
    if (getenv("SSH_CONNECTION") || getenv("SSH_TTY"))
        return 1;

    /* 3. /dev/pts 확인 */
    DIR *d = opendir("/dev/pts");
    if (d) {
        struct dirent *de;
        while ((de = readdir(d)) != NULL) {
            if (de->d_name[0] == '.') continue;
            char path[256];
            snprintf(path, sizeof(path), "/dev/pts/%s", de->d_name);

            struct stat st;
            if (stat(path, &st) == 0 && st.st_uid != 0) {
                closedir(d);
                return 1;
            }
        }
        closedir(d);
    }

    return 0;
}
