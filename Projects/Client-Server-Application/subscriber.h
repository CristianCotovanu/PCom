#ifndef CLIENT_SERVER_SUBSCRIBER_H
#define CLIENT_SERVER_SUBSCRIBER_H

#include "utils.h"

void print_tcp_message(MessageTCP tcp_msg) {
    cout << tcp_msg.ip_addr << ":" << tcp_msg.port << " - "
         << tcp_msg.topic << " - " << tcp_msg.data_type << " - "
         << tcp_msg.data_body << endl;
}

void usage_subscriber() {
    cerr << "Usage: ./subscriber ID_CLIENT IP_SERVER PORT_SERVER" << endl;
    exit(0);
}

#endif
