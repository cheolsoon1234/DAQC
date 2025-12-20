//
// Created by Cheolsoon on 25. 7. 8.
//

#ifndef SERIAL_IO_H
#define SERIAL_IO_H

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

char *UART0_Port = "/dev/ttyAMA0";  // BT module

#define UART_BUFFER_SIZE 256
extern char UART_Buffer[UART_BUFFER_SIZE];

#ifndef CRTSCTS
#define CRTSCTS 020000000000
#endif /* END OF CRTSCTS */

#define SERIAL_MIN_READ_BYTE   0
#define SERIAL_TIMEOUT         1

int32_t serial_open(const char* port);
int32_t serial_configure(int32_t fd, int32_t BaudRate);
int32_t serial_write(int32_t fd, const char* data);
int32_t serial_read(int32_t fd, char* buffer, int32_t MaxLen);
void serial_close(int32_t fd);
int32_t serial_flush(int32_t fd);

#endif //SERIAL_IO_H