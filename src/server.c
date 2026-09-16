#include "protocol.h"
#include "monitor.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct { int fd; struct sockaddr_in peer; } client_ctx_t;

static void *client_thread(void *arg) {
    client_ctx_t *ctx = arg;
    int fd = ctx->fd;
    char ip[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &ctx->peer.sin_addr, ip, sizeof(ip));
    fprintf(stderr, "[+] client connected %s:%d\n", ip, ntohs(ctx->peer.sin_port));
    free(ctx);

    for (;;) {
        char *cmd = NULL;
        uint32_t len = 0;
        if (recv_frame(fd, &cmd, &len) < 0) break;
        char out[1024] = {0};
        if (strcmp(cmd, "quit") == 0) {
            free(cmd);
            break;
        }
        if (monitor_handle(cmd, out, sizeof(out)) < 0)
            snprintf(out, sizeof(out), "ERR failed to read system metrics\n");
        free(cmd);
        if (send_frame(fd, out, (uint32_t)strlen(out)) < 0) break;
    }

    close(fd);
    fprintf(stderr, "[-] client disconnected\n");
    return NULL;
}

static int make_listen_socket(const char *ip, uint16_t port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return -1;
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &a.sin_addr) != 1) { close(s); errno = EINVAL; return -1; }
    if (bind(s, (struct sockaddr *)&a, sizeof(a)) < 0) { close(s); return -1; }
    if (listen(s, 128) < 0) { close(s); return -1; }
    return s;
}

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "Usage: %s <bind_ip> <port>\n", argv[0]); return EXIT_FAILURE; }
    int p = atoi(argv[2]);
    if (p <= 0 || p > 65535) { fprintf(stderr, "Invalid port\n"); return EXIT_FAILURE; }
    signal(SIGPIPE, SIG_IGN);
    int s = make_listen_socket(argv[1], (uint16_t)p);
    if (s < 0) { perror("listen socket"); return EXIT_FAILURE; }
    fprintf(stderr, "Linux monitor server listening on %s:%d\n", argv[1], p);

    for (;;) {
        struct sockaddr_in peer;
        socklen_t n = sizeof(peer);
        int fd = accept(s, (struct sockaddr *)&peer, &n);
        if (fd < 0) { if (errno == EINTR) continue; perror("accept"); break; }
        client_ctx_t *ctx = malloc(sizeof(*ctx));
        if (!ctx) { close(fd); continue; }
        ctx->fd = fd; ctx->peer = peer;
        pthread_t t;
        int rc = pthread_create(&t, NULL, client_thread, ctx);
        if (rc != 0) { fprintf(stderr, "pthread_create failed: %s\n", strerror(rc)); close(fd); free(ctx); continue; }
        pthread_detach(t);
    }
    close(s);
    return EXIT_SUCCESS;
}
