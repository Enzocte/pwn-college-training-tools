#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
    unsigned char dest_mac[6] = {
        0x1e, 0x47, 0x3a, 0xce, 0x88, 0x99
    };

    unsigned char src_mac[6] = {
        0x02, 0x42, 0x0a, 0x22, 0x56, 0x9d
    };

    unsigned char frame[1500];

    for (int i = 0; i < 6 ; i++){
        frame[i] = dest_mac[i]; 
    }

    for (int i = 0; i < 6 ; i++){
        frame[6 + i] = src_mac[i]; 
    }

    frame[12] = 0xFF;
    frame[13] = 0xFF;

    const char payload[] = "HELLO";

    memcpy(&frame[14], payload, sizeof(payload));

    int frame_len = 14 + sizeof(payload);

    int sock = socket(AF_PACKET, SOCK_RAW, htons(0xFFFF));
    if (sock < 0){
        perror("socket");
        return 1;
    }

    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = if_nametoindex("eth0");
    addr.sll_halen = 6;
    memcpy(addr.sll_addr, dest_mac, 6);

    ssize_t sent = sendto(sock, frame, frame_len, 0, (struct sockaddr*)&addr, sizeof(addr)); 

    if (sent < 0){
        perror("sendto");
        return 1;
    }

    close(sock);
    
    return 0;
}
