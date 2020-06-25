#include "include/skel.h"
#include <string>
#include <list>
#include <unordered_map>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <parse_route.h>
#include <iostream>
#include <bits/unordered_map.h>

using namespace std;

TRoute_entry *get_best_route(uint32_t daddr, unordered_map<uint32_t, TRoute_entry> &routing_map) {
    uint32_t mask = UINT32_MAX;

    for (int i = 0; i < 32; i++) {
        if (routing_map.find(daddr & mask) != routing_map.end()) {
            return &(routing_map[daddr & mask]);
        }
        mask >>= 1;
    }
    return nullptr;
}

void forward_icmp(packet &pkt, uint8_t type, uint8_t code) {
    auto *eth_hdr = (struct ether_header *) pkt.payload;
    auto *ip_hdr = (struct iphdr *) (pkt.payload + sizeof(struct ether_header));

    packet icmp_packet = packet();
    auto *eth_hdr_new = (struct ether_header *) icmp_packet.payload;
    auto *ip_hdr_new = (struct iphdr *) (icmp_packet.payload + sizeof(struct ether_header));
    auto *icmp_hdr_new = (struct icmphdr *) (icmp_packet.payload + sizeof(struct ether_header) +
                                             sizeof(struct iphdr));

    icmp_packet.interface = pkt.interface;
    icmp_packet.len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);
    // Eth Header
    eth_hdr_new->ether_type = htons(ETHERTYPE_IP);
    memcpy(eth_hdr_new->ether_dhost, eth_hdr->ether_shost, 6 * sizeof(uint8_t));
    get_interface_mac(pkt.interface, eth_hdr_new->ether_shost);
    // IP Header
    ip_hdr_new->protocol = IPPROTO_ICMP;
    ip_hdr_new->tos = 0;
    ip_hdr_new->frag_off = 0;
    ip_hdr_new->version = 4;
    ip_hdr_new->ihl = 5;
    ip_hdr_new->ttl = 64;
    ip_hdr_new->daddr = ip_hdr->saddr;
    ip_hdr_new->id = htons(getpid());
    ip_hdr_new->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));

    in_addr router_addr;
    inet_aton(get_interface_ip(pkt.interface), &router_addr);
    memcpy(&ip_hdr_new->saddr, &router_addr, 4 * sizeof(struct iphdr));

    ip_hdr_new->check = 0;
    ip_hdr_new->check = ip_checksum(ip_hdr_new, sizeof(struct iphdr));
    //  ICMP Header
    icmp_hdr_new->code = code;
    icmp_hdr_new->type = type;
    icmp_hdr_new->un.echo.sequence = 0;
    icmp_hdr_new->un.echo.id = htons(getpid());
    icmp_hdr_new->checksum = 0;
    icmp_hdr_new->checksum = ip_checksum(icmp_hdr_new, sizeof(struct icmphdr));

    send_packet(icmp_packet.interface, &icmp_packet);
}

using rt_table = std::unordered_map<uint32_t, TRoute_entry>;
using arp_table = std::unordered_map<uint32_t, arp_table_entry>;

void forward_packet(packet &pkt, arp_table &arp_map, rt_table &routing_map) {
    ether_header *eth_hdr = (struct ether_header *) pkt.payload;
    iphdr *ip_hdr = (struct iphdr *) (pkt.payload + sizeof(struct ether_header));

    uint32_t daddr;
    memcpy(&daddr, &ip_hdr->daddr, sizeof(uint32_t));

    uint16_t old_check_sum = ip_hdr->check;
    cout << "RFC 1624 Checksum formula" << endl;
    uint16_t m = (((uint16_t) ip_hdr->protocol) << 8) + ip_hdr->ttl;
    ip_hdr->check = 0;

    if (ip_checksum(ip_hdr, sizeof(struct iphdr)) != old_check_sum) {
        cout << "Package arrived wrong. Different checksum." << endl;
        return;
    }

    TRoute_entry *best_matching_route = get_best_route(daddr, routing_map);

    if (best_matching_route == nullptr) {
        forward_icmp(pkt, ICMP_DEST_UNREACH, ICMP_NET_UNREACH);
        cout << "ICMP Host unreachable" << endl;
        return;
    }

    arp_table_entry best_matching_arp;

    if (arp_map.find(daddr) == arp_map.end()) {
        cout << "ARP Request needed" << endl;
        return;
    } else {
        best_matching_arp = arp_map[daddr];
    }

    ip_hdr->ttl--;
    if (ip_hdr->ttl < 1) {
        forward_icmp(pkt, ICMP_TIME_EXCEEDED, ICMP_EXC_TTL);
        cout << "ICMP Timeout Exceeded, TTL == 0" << endl;
        return;
    }

///    If you uncomment this (basic checksum), comment the next 2 after (RFC 1624 checksum formula)
//    ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));
    uint16_t mp = (((uint16_t) ip_hdr->protocol) << 8) + ip_hdr->ttl;
    ip_hdr->check = ~(~old_check_sum + ~m + 1 + mp);

    pkt.interface = best_matching_route->interface;
    memcpy(eth_hdr->ether_dhost, best_matching_arp.mac_addr, 6 * sizeof(uint8_t));
    get_interface_mac(best_matching_route->interface, eth_hdr->ether_shost);

    send_packet(best_matching_route->interface, &pkt);
}

int main() {
    packet message;
    int received;
    std::string s;

    init();

    FILE *file_pointer;
    file_pointer = fopen("rtable.txt", "r");

    if (file_pointer == nullptr) {
        perror("Routing table file does not exist\n");
        exit(EXIT_FAILURE);
    }

    std::unordered_map<uint32_t, TRoute_entry> routing_map = parse_and_insert(file_pointer);
    fclose(file_pointer);
    std::unordered_map<uint32_t, arp_table_entry> arp_map;
    std::list<packet> packets_waiting_list;

    while (true) {
        received = get_packet(&message);
        DIE(received < 0, "get_message");

        ether_header *eth_hdr = (struct ether_header *) message.payload;

        if (ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP) {
            ether_arp *arp_hdr = (struct ether_arp *) (message.payload + sizeof(struct ether_header));

            if (ntohs(arp_hdr->ea_hdr.ar_op) == ARPOP_REQUEST) {
                uint32_t my_ip;
                inet_pton(AF_INET, get_interface_ip(message.interface), &my_ip);

                if (memcmp(arp_hdr->arp_tpa, &my_ip, sizeof(uint32_t)) == 0) {
                    cout << "Sending ARP_REPLY in response to ARP_REQUEST" << endl;
                    // ETH_Header
                    memcpy(eth_hdr->ether_dhost, eth_hdr->ether_shost, 6 * sizeof(uint8_t));
                    get_interface_mac(message.interface, eth_hdr->ether_shost);
                    eth_hdr->ether_type = htons(ETHERTYPE_ARP);
                    // ARP Header
                    arp_hdr->ea_hdr.ar_hrd = htons(1);
                    arp_hdr->ea_hdr.ar_pro = htons(ETHERTYPE_IP);
                    arp_hdr->ea_hdr.ar_hln = 6;
                    arp_hdr->ea_hdr.ar_pln = 4;
                    arp_hdr->ea_hdr.ar_op = htons(ARPOP_REPLY);

                    memcpy(arp_hdr->arp_tha, arp_hdr->arp_sha, 6 * sizeof(uint8_t));
                    memcpy(arp_hdr->arp_tpa, arp_hdr->arp_spa, 4 * sizeof(uint8_t));
                    get_interface_mac(message.interface, arp_hdr->arp_sha);
                    inet_pton(AF_INET, get_interface_ip(message.interface), &arp_hdr->arp_spa);
                    send_packet(message.interface, &message);
                } else {
                    cout << "Got ARP Request but not for me." << endl;
                    continue;
                }
            } else if (ntohs(arp_hdr->ea_hdr.ar_op) == ARPOP_REPLY) {
                cout << "Got ARP Reply. Update ARP table. Get waiting packet out and send it." << endl;
                uint32_t copy_ip;
                memcpy(&copy_ip, arp_hdr->arp_spa, sizeof(uint32_t));
                arp_table_entry new_arp_entry(copy_ip, arp_hdr->arp_sha);

                if (arp_map.find(copy_ip) == arp_map.end()) {
                    arp_map[copy_ip] = new_arp_entry;
                }

                for (auto it = packets_waiting_list.begin(); it != packets_waiting_list.end(); ++it) {
                    packet &reply_packet = *it;
                    forward_packet(reply_packet, arp_map, routing_map);
                    packets_waiting_list.erase(it);
                    break;
                }
            }
        } else if (ntohs(eth_hdr->ether_type) == ETHERTYPE_IP) {
            iphdr *ip_hdr = (struct iphdr *) (message.payload + sizeof(struct ether_header));

            if (ip_hdr->protocol == IPPROTO_ICMP) {
                cout << "Packet is an ICMP" << endl;
                icmphdr *icmp_hdr = (struct icmphdr *) (message.payload + sizeof(struct ether_header) +
                                                        sizeof(struct iphdr));
                in_addr my_router_addr;
                inet_aton(get_interface_ip(message.interface), &my_router_addr);
                if (icmp_hdr->type == ICMP_ECHO
                    && memcmp(&ip_hdr->daddr, &my_router_addr.s_addr, sizeof(uint32_t)) == 0) {
                    cout << "Sending ICMP Echo Reply" << endl;
                    forward_icmp(message, ICMP_ECHOREPLY, ICMP_ECHOREPLY);
                }
            }

            uint32_t old_check_sum = ip_hdr->check;
            ip_hdr->check = 0;
            if (ip_checksum(ip_hdr, sizeof(struct iphdr)) != old_check_sum) {
                cout << "Package is wrong, checksum differs" << endl;
                continue;
            }
            ip_hdr->check = old_check_sum;

            route_table_entry *best_matching_route = get_best_route(ip_hdr->daddr, routing_map);
            if (best_matching_route == nullptr) {
                cout << "Sending ICMP Host Unreachable" << endl;
                forward_icmp(message, ICMP_DEST_UNREACH, ICMP_NET_UNREACH);
                continue;
            } else {
                uint32_t copy_ip;
                memcpy(&copy_ip, &ip_hdr->daddr, sizeof(uint32_t));

                if (arp_map.find(copy_ip) == arp_map.end()) {
                    cout << "Cannot find ARP Entry. Sending ARP Request. Put packet into the waiting list" << endl;
                    packet new_arp_request = packet();
                    ether_header *new_eth_hdr = (struct ether_header *) new_arp_request.payload;
                    ether_arp *new_arp_hdr = (struct ether_arp *) (new_arp_request.payload +
                                                                   sizeof(struct ether_header));

                    // Packet
                    new_arp_request.len = sizeof(ether_header) + sizeof(ether_arp);
                    new_arp_request.interface = best_matching_route->interface;
                    //  Eth Header
                    memset(new_eth_hdr->ether_dhost, 0xff, ETH_ALEN * sizeof(uint8_t));
                    get_interface_mac(best_matching_route->interface, new_eth_hdr->ether_shost);
                    new_eth_hdr->ether_type = htons(ETHERTYPE_ARP);
                    //  ARP Header
                    new_arp_hdr->ea_hdr.ar_hrd = htons(1);
                    new_arp_hdr->ea_hdr.ar_pro = htons(ETHERTYPE_IP);
                    new_arp_hdr->ea_hdr.ar_hln = 6;
                    new_arp_hdr->ea_hdr.ar_pln = 4;
                    new_arp_hdr->ea_hdr.ar_op = htons(ARPOP_REQUEST);
                    get_interface_mac(best_matching_route->interface, new_arp_hdr->arp_sha);
                    inet_pton(AF_INET, get_interface_ip(best_matching_route->interface), new_arp_hdr->arp_spa);
                    memset(new_arp_hdr->arp_tha, 0x00, ETH_ALEN * sizeof(uint8_t));
                    memcpy(new_arp_hdr->arp_tpa, &best_matching_route->next_hop, 4 * sizeof(uint8_t));

                    cout << "Adding packet in waiting list." << endl;
                    packets_waiting_list.push_front(message);

                    send_packet(new_arp_request.interface, &new_arp_request);
                } else {
                    forward_packet(message, arp_map, routing_map);
                }
            }
        }
    }
    return 0;
}
