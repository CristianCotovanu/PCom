#ifndef ROUTER_PARSE_ROUTE_H
#define ROUTER_PARSE_ROUTE_H

#include <cstdint>
#include <map>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

struct arp_table_entry {
    uint32_t ip_addr;
    unsigned char mac_addr[6];

    arp_table_entry() = default;

    arp_table_entry(const arp_table_entry &arpTable_entry) = default;

    arp_table_entry (uint32_t ip_addr, unsigned char mac_addr[6]) {
        this->ip_addr = ip_addr;
        memcpy(this->mac_addr, mac_addr, 6 * sizeof(unsigned char));
    }
};

typedef struct route_table_entry {
    uint32_t prefix;
    uint32_t next_hop;
    uint32_t mask;
    int interface;

    route_table_entry() = default;

    route_table_entry(const route_table_entry &routeTable_entry) = default;

    bool operator < (const route_table_entry &routeTable_entry) {
        return this->prefix > routeTable_entry.prefix;
    }

    bool operator == (const route_table_entry &routeTable_entry) {
        return (this->prefix == routeTable_entry.prefix)
            && (this->interface == routeTable_entry.interface)
            && (this->mask == routeTable_entry.mask)
            && (this->next_hop == routeTable_entry.next_hop);
    }
} TRoute_entry;

char* string_to_char(const std::string& str) {
    int string_size = str.size();
    char* to_char = (char* )calloc(string_size + 1, sizeof(char));
    str.copy(to_char, string_size + 1);
    to_char[string_size] = '\0';

    return to_char;
}

std::vector<std::string> split (const std::string& s, const std::string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

std::unordered_map<uint32_t , TRoute_entry> parse_and_insert(FILE* file_pointer) {
    std::unordered_map<uint32_t, TRoute_entry> routing_map;

    char *line = nullptr;
    size_t len = 0;

    while ((getline(&line, &len, file_pointer)) != -1) {
        std::vector<std::string> routing_line = split(line, " ");

        TRoute_entry rtable_entry;
        char* to_char = string_to_char(routing_line.at(0));
        inet_pton(AF_INET, to_char, &rtable_entry.prefix);
        to_char = string_to_char(routing_line.at(1));
        inet_pton(AF_INET, to_char, &rtable_entry.next_hop);
        to_char = string_to_char(routing_line.at(2));
        inet_pton(AF_INET, to_char, &rtable_entry.mask);
        to_char = string_to_char(routing_line.at(3));
        rtable_entry.interface = atoi(to_char);

        routing_map[rtable_entry.prefix] = rtable_entry;
    }

    return routing_map;
}

#endif //ROUTER_PARSE_ROUTE_H
