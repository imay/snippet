#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int create_listening(const char* host, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sockaddr;
    int optval = 1;
    socklen_t socklen;
    if (fd < 0) {
        perror("socket failed: ");
        goto fail;
    }
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    socklen = sizeof(sockaddr);
    memset(&sockaddr, 0, socklen);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    if (host == NULL) {
        sockaddr.sin_addr.s_addr = INADDR_ANY;
    } else {
        sockaddr.sin_addr.s_addr = inet_addr(host);
    }

    if (bind(fd, (struct sockaddr*)&sockaddr, socklen)) {
        perror("bind failed");
        goto fail;
    }

    if (listen(fd, 1024)) {
        perror("listen failed");
        goto fail;
    }
    fprintf(stdout, "listen on %s:%d\n", host, port);

    return fd;
fail:
    if (fd >= 0) {
        close(fd);
    }
    return -1;
}

void loop(int fd) {
    struct sockaddr_in sockaddr;
    socklen_t socklen = sizeof(sockaddr);
    int client_fd = accept(fd, (struct sockaddr*)&sockaddr, &socklen);
    if (client_fd < 0) {
        perror("accept failed");
        exit(-1);
    }
    while (1) {
        char buf[256];
        int len = recv(client_fd, buf, 256, 0);
        if (strncmp(buf, "quit", 4) == 0) {
            close(client_fd);
            break;
        }
        send(client_fd, buf, len, 0);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s host ip\n", argv[0]);
        exit(-1);
    }
    int fd = create_listening(argv[1], atol(argv[2]));
    if (fd < 0) {
        exit(-1);
    }
    loop(fd);
    close(fd);
    return 0;
}

