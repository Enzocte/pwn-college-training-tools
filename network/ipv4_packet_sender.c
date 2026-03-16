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

    unsigned char ver_length[1] = { 0x45};

    unsigned char service_field[1] = {0x00};

    unsigned char total_lenght[2] = {0x00, 0x14};

    unsigned char indent[2] = {0x00, 0x00};

    unsigned char flag_fragOffset[2] = {0x40, 0x00};

    unsigned char time_to_live[1] = {0x40};

    unsigned char protocol[1] = {0xFF};

    unsigned char header_checksum[2] = {0x25, 0xE9};

    unsigned char src_ip[4] = {0x0a, 0x00, 0x00, 0x01};

    unsigned char dest_ip[4] = {0x0a, 0x00, 0x00, 0x02};

    unsigned char frame[1500];

    for (int i = 0; i < 6 ; i++){
        frame[i] = dest_mac[i]; 
    }

    for (int i = 0; i < 6 ; i++){
        frame[6 + i] = src_mac[i]; 
    }

    frame[12] = 0x08;
    frame[13] = 0x00;
    frame[14] = ver_length[0];
    frame[15] = service_field[0];
    frame[16] = total_lenght[0];
    frame[17] = total_lenght[1];
    frame[18] = indent[0];
    frame[19] = indent[1];
    frame[20] = flag_fragOffset[0];
    frame[21] = flag_fragOffset[1];
    frame[22] = time_to_live[0];
    frame[23] = protocol[0];
    frame[24] = header_checksum[0];
    frame[25] = header_checksum[1];
    
    for (int i = 0; i < 4; i++){
        frame[26 + i] = src_ip[i];
    }

    for (int i = 0; i < 4; i++) {
      frame[30 + i] = dest_ip[i];
    }
    /**
    const char payload[] = "HELLO";
    memcpy(&frame[34], payload, sizeof(payload));
    */
    int frame_len = 34;

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
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
