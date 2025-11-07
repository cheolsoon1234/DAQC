//
// Created by cheolsoon on 25. 7. 28.
//
#include "gpio_handler.h"
#include "sysfs_handler.h"
#include <unistd.h>

int32_t GPIO_Export(int32_t gpio) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", gpio);

    if (!write_sysfs("/sys/class/gpio/export", buffer)) {
        fprintf(stderr, "[WARN] ERROR while exporting GPIO %d\n", gpio);
        return 0;
    }

    usleep(100000); // 커널 반영 대기
    return 1;
}

int32_t GPIO_Unexport(int32_t gpio) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", gpio);

    if (!write_sysfs("/sys/class/gpio/unexport", buffer)) {
        fprintf(stderr, "[WARN] ERROR while unexporting GPIO %d\n", gpio);
        return 0;
    }

    return 1;
}

int32_t GPIO_Set_Direction(int32_t gpio, const char* direction) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio);

    if (!write_sysfs(path, direction)) {
        fprintf(stderr, "[ERROR] Failed to set direction for GPIO %d\n", gpio);
        return 0;
    }

    return 1;
}

int32_t GPIO_Write(int32_t gpio, int32_t value) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);

    const char* val_str = value ? "1" : "0";

    if (!write_sysfs(path, val_str)) {
        fprintf(stderr, "[ERROR] Failed to write value (%s) to GPIO %d\n", val_str, gpio);
        return 0;
    }

    return 1;
}