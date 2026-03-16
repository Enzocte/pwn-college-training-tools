#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int main(void) {

    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0){
        perror("socket");
        return 1;
    }

    int ifidx = if_nametoindex("eth0");
    if (ifidx == 0){
        perror("idx");
        return 1;
    }

    unsigned char my_mac[6] = {0xc2, 0x84, 0xbf, 0xea, 0x25, 0xd1};
    unsigned char mac2[6] = {0xbe, 0x4d, 0x4d, 0xe4, 0xe8, 0x86};
    unsigned char mac3[6] = {0xb6, 0xaf, 0xd9, 0xa3, 0x68, 0xeb};
    uint32_t ip2 = inet_addr("10.0.0.2");
    uint32_t ip3 = inet_addr("10.0.0.3");

    struct sockaddr_ll to = {0};
    to.sll_family = AF_PACKET;
    to.sll_ifindex = ifidx;
    to.sll_halen = 6;

    unsigned char buf[2000];
    for (;;) {
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
        if (n < (ssize_t)(sizeof(struct ethhdr) + sizeof(struct iphdr)))
            continue;

        struct ethhdr *eth = (struct ethhdr *)buf;
        if (memcmp(eth->h_source, my_mac, 6) == 0)
            continue;
        if (ntohs(eth->h_proto) != ETH_P_IP)
            continue;

        struct iphdr *ip = (struct iphdr *)(buf + sizeof(struct ethhdr));

	char a[16], b[16];
	inet_ntop(AF_INET, &ip->saddr, a, sizeof(a));
	inet_ntop(AF_INET, &ip->daddr, b, sizeof(b));
	fprintf(stderr, "IP %s -> %s\n", a, b);

        if (ip->saddr == ip2 && ip->daddr == ip3){
          memcpy(eth->h_dest, mac3, 6);
          memcpy(eth->h_source, my_mac, 6);
          memcpy(to.sll_addr, mac3, 6);
	  fprintf(stderr, "2->3\n");
        }

        else if (ip->saddr == ip3 && ip->daddr == ip2) {
          memcpy(eth->h_dest, mac2, 6);
          memcpy(eth->h_source, my_mac, 6);
          memcpy(to.sll_addr, mac2, 6);
	  fprintf(stderr, "3->2\n");
        }

        else{
            continue;
        }

        ssize_t sent = sendto(fd, buf, n, 0,
                              (struct sockaddr *)&to, sizeof(to));

        if (sent < 0) {
          perror("sendto");
          return 1;
        }
    }
}
