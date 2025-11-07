#ifndef __NET_H__
#define __NET_H__

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <libwebsockets.h>
#include <stdint.h>

#include "signal_handler.h"

extern struct lws_context *context;

// #define SERVER_ADDRESS "172.17.88.75"
// #define SERVER_ADDRESS "13.125.103.172"
// #define SERVER_ADDRESS "192.168.0.90"
#define SERVER_ADDRESS "192.168.108.42"
#define SERVER_PORT 3304


// 타임아웃/하트비트 설정 (밀리초 단위)
#define HEARTBEAT_INTERVAL_MS 100      // 핑 전송 주기
#define CONNECTION_TIMEOUT_MS 1000     // 응답 없을 때 타임아웃 (1s)


int32_t init_ws(void);

void send_json(double Pre1, double Pre2, double tmp, double thrust);
void net_stop(void);

void ws_exit_handler(int sig);
// 동기 처리 API: 주기적으로 호출하여 이벤트를 처리
int net_poll(int timeout_ms);

#endif // __NET_H__
