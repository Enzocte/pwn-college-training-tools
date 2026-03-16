#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(void) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

struct sockaddr_in local;
memset(&local, 0, sizeof(local));
local.sin_family = AF_INET;
local.sin_port = htons(31337);
local.sin_addr.s_addr = htonl(INADDR_ANY);

if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
    perror("bind");
    close(sock);
    return 1;
}

for (int i = 1; i < 65536; i++){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(i);
    if (inet_pton(AF_INET, "10.0.0.2", &addr.sin_addr) != 1) {
        perror("inet_pton");
        close(sock);
        return 1;
    }
    char msg[64];
    int flag_port = 4444;
    snprintf(msg, sizeof(msg), "FLAG:10.0.0.1:%d", flag_port);

    ssize_t sent = sendto(sock, msg, strlen(msg), 0,
                          (struct sockaddr *)&addr, sizeof(addr));
    if (sent < 0) {
        printf("pas le port  %d", i);
        
    }}
    close(sock);

    return 0;
}
