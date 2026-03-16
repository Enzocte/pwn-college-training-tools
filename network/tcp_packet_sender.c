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
#include <netinet/tcp.h>

int main(){
    //Partie Ethernet
    unsigned char dest_mac[6] = {
        0x1e, 0x47, 0x3a, 0xce, 0x88, 0x99
    };
    unsigned char src_mac[6] = {
        0x02, 0x42, 0x0a, 0x22, 0x56, 0x9d
    };

    //Partie Ip
    unsigned char ver_length[1] = { 0x45};
    unsigned char service_field[1] = {0x00};
    unsigned char total_lenght[2] = {0x00, 0x28};
    unsigned char indent[2] = {0x00, 0x00};
    unsigned char flag_fragOffset[2] = {0x40, 0x00};
    unsigned char time_to_live[1] = {0x40};
    unsigned char protocol[1] = {0x06};
    unsigned char header_checksum[2] = {0x25, 0xE9};
    unsigned char src_ip[4] = {0x0a, 0x00, 0x00, 0x01};
    unsigned char dest_ip[4] = {0x0a, 0x00, 0x00, 0x02};

    //Partie Tcp
    /*
        Manually send a Transmission Control Protocol packet. The packet should
        have TCP sport=31337, dport=31337, seq=31337, ack=31337, flags=APRSF.
        The packet should be sent to the remote host at 10.0.0.2.
    */

    unsigned char src_port[2] = {0x7A, 0x69};
    unsigned char dest_port[2] = {0x7A, 0x69};
    unsigned char seq[4] = {0x00, 0x00, 0x7A, 0x69};
    unsigned char ack[4] = {0x00, 0x00, 0x00, 0x00};
    unsigned char dataOff_res_flags[2] = {0x50, 0x02};
    unsigned char window[2] = {0x20, 0x00};
    unsigned char checksum[2] = {0x2C, 0x50};
    unsigned char urg_pointer[2] = {0x00, 0x00};

    //Partie alocation du packet
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

    frame[34] = src_port[0];
    frame[35] = src_port[1];
    frame[36] = dest_port[0];
    frame[37] = dest_port[1];
    for (int i = 0; i < 4; i++){
        frame[38 + i] = seq[i];
    }
    for (int i = 0; i < 4; i++) {
      frame[42 + i] = ack[i];
    }
    frame[46] = dataOff_res_flags[0];
    frame[47] = dataOff_res_flags[1];
    frame[48] = window[0];
    frame[49] = window[1];
    frame[50] = checksum[0];
    frame[51] = checksum[1];
    frame[52] = urg_pointer[0];
    frame[53] = urg_pointer[1];

    /**
    const char payload[] = "HELLO";
    memcpy(&frame[34], payload, sizeof(payload));
    */
    int frame_len = 54;

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

    if (sent < 0) {
        perror("sendto");
        return 1;
    }

    unsigned char recv_buf[1500];
    struct sockaddr_ll recv_addr;
    socklen_t recv_addrlen = sizeof(recv_addr);

    ssize_t r = recvfrom(sock, recv_buf, sizeof(recv_buf),
                         0,
                         (struct sockaddr *)&recv_addr, &recv_addrlen);

    if (r < 0) {
      perror("recvfrom");
      return 1;
    }

    struct pseudo {
      uint32_t saddr;
      uint32_t daddr;
      uint8_t zero;
      uint8_t proto;
      uint16_t tcp_len;
    };

    uint16_t csum(uint16_t *buf, int words) {
      uint32_t sum = 0;
      for (int i = 0; i < words; i++)
        sum += ntohs(buf[i]);
      while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
      return htons(~sum);
    }

    if (r >= 0){
        printf("recu %zd octets\n", r);

        struct tcphdr *th = (struct tcphdr *)(recv_buf + 34);

        uint32_t server_seq = ntohl(th->seq);
        uint32_t server_ack = ntohl(th->ack_seq);

        uint32_t my_seq = htonl(server_ack);
        uint32_t my_ack = htonl(server_seq + 1);

        memcpy(&frame[38], &my_seq, 4);
        memcpy(&frame[42], &my_ack, 4);

        frame[47] = 0x10;
        frame[50] = frame[51] = 0;

        ssize_t sent = sendto(sock, frame, frame_len, 0, (struct sockaddr *)&addr,
                                sizeof(addr));

        if (sent < 0) {
            perror("sendto");
            return 1;
        }
    }

    close(sock);

    return 0;
}
