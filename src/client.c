#include "protocol.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CMD_LEN 256

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]); return EXIT_FAILURE; }
    int port = atoi(argv[2]);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return EXIT_FAILURE; }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, argv[1], &a.sin_addr) != 1) { fprintf(stderr, "Invalid IP address\n"); close(fd); return EXIT_FAILURE; }
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) < 0) { perror("connect"); close(fd); return EXIT_FAILURE; }

    printf("Connected to Linux monitor at %s:%d\n", argv[1], port);
    printf("Commands: cpu, memory, uptime, load, status, help, quit\n\n");
    char line[MAX_CMD_LEN];
    while (printf("> "), fflush(stdout), fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';
        if (!len) continue;
        if (send_frame(fd, line, (uint32_t)len) < 0) { perror("send_frame"); break; }
        if (strcmp(line, "quit") == 0) break;
        char *response = NULL;
        uint32_t response_len = 0;
        if (recv_frame(fd, &response, &response_len) < 0) { fprintf(stderr, "Server disconnected or protocol error\n"); break; }
        printf("%s", response);
        free(response);
    }
    close(fd);
    return EXIT_SUCCESS;
}
