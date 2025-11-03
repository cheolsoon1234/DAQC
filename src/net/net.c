#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "net.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <sys/poll.h>
#include <errno.h>

// --------------------------- // 전역 변수 ---------------------------
struct lws_context *context = NULL;
extern volatile sig_atomic_t g_stop; // 전역 신호 플래그는 core에서 관리
static struct lws *ws_recv_wsi = NULL;
static struct lws *ws_send_wsi = NULL;
static char pending_message[512];
static int32_t has_pending_message = 0;
// 외부에서 참조하는 제어 입력 배열
volatile int32_t controls[7] = {0};

// =========================== // 수신용 WebSocket 콜백 (/ws-rpi) ===========================
static int32_t callback_ws_recv(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lwsl_user("Connected to /ws-rpi\n");
            LOG_WRITE_WITH_TIME("[INFO], Connected to %s/ws-rpi", SERVER_ADDRESS);
            ws_recv_wsi = wsi;
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            printf("[WS RECV] %.*s\n", (int)len, (char*)in);
            fflush(stdout);
            sscanf((char*)in, "%d,%d,%d,%d,%d,%d,%d",
                   &controls[0], &controls[1], &controls[2],
                   &controls[3], &controls[4], &controls[5],
                   &controls[6]);
	    LOG_WRITE_WITH_TIME("[WS_RECV],%d,%d,%d,%d,%d,%d,%d", &controls[0], &controls[1], &controls[2], &controls[3], &controls[4], &controls[5], &controls[6]);
            break;
        }
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            LOG_WRITE_WITH_TIME("[ERROR], Connection error %s/ws-rpi", SERVER_ADDRESS);
            printf("[ERROR] Connection error %s/ws-rpi\n", SERVER_ADDRESS);
            exit(1);
        case LWS_CALLBACK_CLOSED:
            lwsl_user("Disconnected from /ws-rpi\n");
            ws_recv_wsi = NULL;
            LOG_WRITE_WITH_TIME("[INFO], Disconnected from %s/ws-rpi", SERVER_ADDRESS);
            printf("[INFO] Disconnected from %s/ws-rpi\n", SERVER_ADDRESS);
            exit(1);
        default:
            break;
    }
    return 0;
}

// =========================== // 송신용 WebSocket 콜백 (/ws-addData) ===========================
static int32_t callback_ws_send(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lwsl_user("Connected to /ws-addData\n");
            LOG_WRITE_WITH_TIME("[INFO], Connected to %s/ws-addData", SERVER_ADDRESS);
            ws_send_wsi = wsi;
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            lwsl_err("Connection error %s/ws-addData\n", SERVER_ADDRESS);
            ws_send_wsi = NULL;
            LOG_WRITE_WITH_TIME("[ERROR], Connection error %s/ws-addData", SERVER_ADDRESS);
            printf("[ERROR] Connection error %s/ws-addData\n", SERVER_ADDRESS);
            exit(1);
        case LWS_CALLBACK_CLOSED:
            lwsl_user("Disconnected from %s/ws-addData\n", SERVER_ADDRESS);
            ws_send_wsi = NULL;
            LOG_WRITE_WITH_TIME("[INFO], Disconnected from %s/ws-addData", SERVER_ADDRESS);
            printf("[INFO] Disconnected from %s/ws-addData\n", SERVER_ADDRESS);
            exit(1);
        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            if (has_pending_message && ws_send_wsi) {
                unsigned char buf[LWS_PRE + 512];
                unsigned char *p = &buf[LWS_PRE];
                size_t msg_len = strlen(pending_message);
                memcpy(p, pending_message, msg_len);
                int32_t n = lws_write(wsi, p, msg_len, LWS_WRITE_TEXT);
                if (n < (int)msg_len)
                    lwsl_err("lws_write error\n");
                has_pending_message = 0;
            }
            lws_callback_on_writable(wsi);
            break;
        }
        default:
            break;
    }
    return 0;
}

// =========================== // 프로토콜 정의 ===========================
static const struct lws_protocols protocols[] = { 
    { "ws-rpi", callback_ws_recv, 0, 512 }, 
    { "ws-addData", callback_ws_send, 0, 512 }, 
    { NULL, NULL, 0, 0 } 
}; 

// =========================== // WebSocket 연결 ===========================
static struct lws *connect_ws(const char *address, int32_t port, const char *path, const char *protocol) {
    struct lws_client_connect_info ccinfo;
    memset(&ccinfo, 0, sizeof(ccinfo));
    ccinfo.context = context;
    ccinfo.address = address;
    ccinfo.port = port;
    ccinfo.path = path;
    ccinfo.host = address;
    ccinfo.origin = "origin";
    ccinfo.protocol = protocol;
    struct lws *wsi = lws_client_connect_via_info(&ccinfo);
    if (!wsi) lwsl_err("Connection failed: %s%s\n", address, path);
    return wsi;
}

// =========================== // JSON 전송 함수 ===========================
void send_json(double Pre1, double Pre2, double tmp, double thrust) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm_info;
    localtime_r(&ts.tv_sec, &tm_info);
    char created_at[64];
    strftime(created_at, sizeof(created_at), "%Y-%m-%d %H:%M:%S", &tm_info);
    snprintf(created_at + strlen(created_at), sizeof(created_at) - strlen(created_at), ".%06ld", ts.tv_nsec / 1000);

    snprintf(pending_message, sizeof(pending_message),
             "{\"val1\": %.6f, \"val2\": %.6f, \"val3\": %.6f, \"val4\": %.6f, \"created_at\": \"%s\"}",
             Pre1, Pre2, tmp, thrust, created_at);
    has_pending_message = 1;
    if (ws_send_wsi)
        lws_callback_on_writable(ws_send_wsi);
}

// =========================== // WebSocket 초기화 ===========================
int32_t init_ws(void) {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    lws_set_log_level(LLL_USER | LLL_ERR, NULL);
    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "lws init failed\n");
        return -1;
    }

    connect_ws(SERVER_ADDRESS, SERVER_PORT, "/ws-rpi", protocols[0].name);
    connect_ws(SERVER_ADDRESS, SERVER_PORT, "/ws-addData", protocols[1].name);
    return 0;
}

// =========================== // FD 기반 연결 확인 ===========================
static int check_fd(struct lws *wsi) {
    if (!wsi) return -1;
    int fd = lws_get_socket_fd(wsi);
    if (fd < 0) return -1;

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, 0);  // timeout 없이 즉시 반환
    if (ret > 0) {
        if (pfd.revents & (POLLHUP | POLLERR)) return -1;
    } else if (ret < 0) {
        return -1;
    }
    return 0;
}

// =========================== // net_poll ===========================
int32_t net_poll(int32_t timeout_ms) {
    if (!context) return -1;

    // ws-rpi FD 체크
    if (ws_recv_wsi && check_fd(ws_recv_wsi) != 0) {
        lwsl_err("ws-rpi connection lost\n");
        exit(1);
    }

    // ws-addData FD 체크
    if (ws_send_wsi && check_fd(ws_send_wsi) != 0) {
        lwsl_err("ws-addData connection lost\n");
        exit(1);
    }

    return lws_service(context, timeout_ms);
}

// =========================== // 종료 핸들러 ===========================
void ws_exit_handler(int32_t sig) {
    (void)sig;
    if (context) lws_cancel_service(context);
}

void net_stop(void) {
    if (context) {
        lws_cancel_service(context);
        lws_context_destroy(context);
        context = NULL;
        ws_recv_wsi = NULL;
        ws_send_wsi = NULL;
        has_pending_message = 0;
    }
}
