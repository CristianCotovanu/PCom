#ifndef CLIENT_SERVER_SERVER_H
#define CLIENT_SERVER_SERVER_H

#include <cmath>
#include "utils.h"

#define SERVER_MAX_INPUT_LEN 10
#define MAX_CLIENTS    5
#define BUFF_LEN 1501

struct MessageUDP {
    char topic[TOPIC_MAX_LEN];
    uint8_t data_type;
    char data_body[BODY_MAX_LEN];

    MessageUDP() = default;
};

struct OfflineClient {
    ClientTCP client;
    bool online;
    bool exists;
};

void close_client(int sock, fd_set &read_fds, vector<ClientTCP> &tcp_clients) {
    for (int i = 0; i < tcp_clients.size(); i++) {
        if (tcp_clients[i].sockd == sock) {
            tcp_clients[i].sockd = -1;
            tcp_clients[i].online = false;
            cout << "Client " << tcp_clients[i].id << " disconnected." << endl;
            close(sock);
            FD_CLR(sock, &read_fds);
            break;
        }
    }
}

OfflineClient find_offline_client(const vector<ClientTCP> &clients, char *search) {
    OfflineClient ret;
    ret.online = false;
    ret.exists = false;

    for (ClientTCP clientTcp : clients) {
        if (strcmp(clientTcp.id, search) == 0) {
            ret.client = clientTcp;
            ret.exists = true;

            if (clientTcp.online) {
                ret.online = true;
            }
            break;
        }
    }

    return ret;
}

void send_error_message(int sockfd, int port, char *description) {
    MessageTCP msg{};

    msg.port = port;
    strcpy(msg.data_body, description);
    strcpy(msg.topic, "\0");
    strcpy(msg.ip_addr, "\0");
    send(sockfd, &msg, sizeof(MessageTCP), 0);
}


MessageTCP udp_to_tcp(MessageUDP udp_msg) {
    MessageTCP tcp_msg{};

    if (udp_msg.data_type == 0) {
        int tmp;
        uint8_t sign;
        memcpy(&sign, udp_msg.data_body, sizeof(uint8_t));
        memcpy(&tmp, udp_msg.data_body + 1, sizeof(int));
        tmp = ntohl(tmp);
        sign == 1 ? tmp *= -1 : 0;

        sprintf(tcp_msg.data_body, "%d", tmp);
        strcpy(tcp_msg.data_type, "INT\0");
    } else if (udp_msg.data_type == 1) {
        double tmp;
        tmp = ntohs(*(uint16_t *) udp_msg.data_body);
        tmp /= 100;

        sprintf(tcp_msg.data_body, "%.2f", tmp);
        strcpy(tcp_msg.data_type, "SHORT_REAL\0");
    } else if (udp_msg.data_type == 2) {
        float tmp;
        uint8_t sign;
        memcpy(&sign, udp_msg.data_body, sizeof(uint8_t));
        tmp = ntohl(*(uint32_t *) (udp_msg.data_body + 1));
        tmp /= pow(10, udp_msg.data_body[5]);
        sign == 1 ? tmp *= -1 : 0;

        sprintf(tcp_msg.data_body, "%lf", tmp);
        strcpy(tcp_msg.data_type, "FLOAT\0");
    } else if (udp_msg.data_type == 3) {
        memcpy(tcp_msg.data_body, udp_msg.data_body, strlen(udp_msg.data_body) + 1);
        strcpy(tcp_msg.data_type, "STRING\0");
    }

    memcpy(&tcp_msg.topic, &udp_msg.topic, strlen(udp_msg.topic) + 1);
    return tcp_msg;
}

void usage_server() {
    cerr << "Usage: ./server PORT_SERVER" << endl;
    exit(0);
}


#endif
