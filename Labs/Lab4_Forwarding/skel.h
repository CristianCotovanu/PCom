#pragma once

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
/* According to POSIX.1-2001, POSIX.1-2008 */
#include <sys/select.h>
/* ethheader */
#include <net/ethernet.h>
/* ether_header */
#include <arpa/inet.h>
/* icmphdr */
#include <netinet/ip_icmp.h>
/* arphdr */
#include <net/if_arp.h>
#include <asm/byteorder.h>
#include "parser.h"

/* 
 *Note that "buffer" should be at least the MTU size of the 
 * interface, eg 1500 bytes 
 */
#define MAX_LEN 1600
#define ROUTER_NUM_INTERFACES 4

#define DIE(condition, message) \
	do { \
		if ((condition)) { \
			fprintf(stderr, "[%d]: %s\n", __LINE__, (message)); \
			perror(""); \
			exit(1); \
		} \
	} while (0)

typedef struct {
	int len;
	char payload[MAX_LEN];
	int interface;
} msg;

struct arp_entry {
	__u32 ip;
	uint8_t mac[6];
};

//struct    ether_header {
//    u_char     ether_dhost[6]; // mac_destinatie
//    u_char     ether_shost[6]; // mac_sursa
//    u_short    ether_type;
//};
//
//struct iphdr {
//#if defined(__LITTLE_ENDIAN_BITFIELD)
//    __u8    ihl:4,
//            version:4;
//#elif defined (__BIG_ENDIAN_BITFIELD)
//    __u8    version:4,
//          ihl:4;
//#else
//#error    "Please fix <asm/byteorder.h>"
//#endif
//    __u8    tos;
//    __u16    tot_len;
//    __u16    id;
//    __u16    frag_off;
//    __u8    ttl; // se decrementeaza
//    __u8    protocol;
//    __u16    check;
//    __u32    saddr; // adresa IP SURSA
//    __u32    daddr; // adresa IP DESTINATIE
//    /*The options start here. */
//};
//
//ip_checksum(ip_hdr, sizeof(struct iphdr));
//
//struct route_table_entry {
//    uint32_t prefix;
//    uint32_t next_hop;
//    uint32_t mask;
//    int interface;
//} __attribute__((packed));
//
//
//struct arp_entry {
//    __u32 ip;
//    uint8_t mac[6];
//};



extern int interfaces[ROUTER_NUM_INTERFACES];

int send_packet(int sockfd, msg *m);
int get_packet(msg *m);
char *get_interface_ip(int interface);
int get_interface_mac(int interface, uint8_t *mac);
uint16_t ip_checksum(void* vdata,size_t length);
void init();
void parse_arp_table();
/**
 * hwaddr_aton - Convert ASCII string to MAC address (colon-delimited format)
 * @txt: MAC address as a string (e.g., "00:11:22:33:44:55")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
int hwaddr_aton(const char *txt, uint8_t *addr);

