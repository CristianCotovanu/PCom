void print_ip(uint32_t ip) {
    unsigned char bytes[4];
    bytes[3] = ip & 0xFF;
    bytes[2] = (ip >> 8) & 0xFF;
    bytes[1] = (ip >> 16) & 0xFF;
    bytes[0] = (ip >> 24) & 0xFF;
    cout << bytes[3] << "." << bytes[2] << "." << bytes[1] <<  "." << bytes[0] << endl;
    cerr << bytes[3] << "." << bytes[2] << "." << bytes[1] <<  "." << bytes[0] << endl;
}

void print_Ip_Pieces(uint8_t ip[4]) {
    unsigned char bytes[4];
    bytes[3] = ip[0];
    bytes[2] = ip[1];
    bytes[1] = ip[2];
    bytes[0] = ip[3];
    cout << bytes[3] << "." << bytes[2] << "." << bytes[1] <<  "." << bytes[0] << endl;
}

void print_MAC(uint8_t *mac) {
    cout << hex << mac[0] << ":" << mac[1] << ":" << mac[2] <<  ":" << mac[3] << ":" << mac[4] <<  ":" << mac[5] << endl;
}

void print_arp_table(std::unordered_map<uint32_t, arp_table_entry> arp_map) {
    cout << "PRINTARE TABELA ARP\n";
    for (auto & it : arp_map) {
        cout << "MAC ";
        printMAC(it.second.mac_addr);
        cout << "IP ";
        printIp(it.second.ip_addr);
        cout << endl;
    }
    cout << "AM TERMINAT DE PRINTAT TABELA ARP\n\n";
}

void print_routing_table(const std::unordered_map<uint32_t, TRoute_entry>& routing_map) {
    cout << "PRINTARE TABELA RUTARE\n";
    for (auto & it : routing_map) {
        cout << "Next hop: ";
        printIp(it.second.next_hop);
        cout << "Prefix: ";
        printIp(it.second.prefix);
        cout << "Mask: ";
        printIp(it.second.mask);
        cout <<"Intefata:" << it.second.interface << endl << endl;

        if (it.second.prefix == 0) {
            cout << "WTFWTFWTF DE CE E PREFIXUL 0";
        }
    }
    cout << "AM TERMINAT DE PRINTAT TABELA DE RUTARE\n";
}
