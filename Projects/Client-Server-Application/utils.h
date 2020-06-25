#ifndef CLIENT_SERVER_UTILS_H
#define CLIENT_SERVER_UTILS_H

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define TCP_CLIENT_INPUT_LEN 50
#define ID_MAX_LEN 11
#define BODY_MAX_LEN 1501
#define TOPIC_MAX_LEN 50
#define IP_ADDR_LEN 16
#define CLOSE_CLIENT -1
#define DUMMY_FLAG -7
#define DUMMY_MSG_LEN 15
#define LONG_MSG_LEN 30

using namespace std;

struct MessageTCP {
    int port;
    char ip_addr[IP_ADDR_LEN];
    char topic[TOPIC_MAX_LEN];
    char data_type[ID_MAX_LEN];
    char data_body[BODY_MAX_LEN];

    MessageTCP() = default;
};

struct Topic {
    char name[50];
    int sf;

    Topic() = default;

    Topic(char *name) {
        strcpy(this->name, name);
    }
};

struct ClientTCP {
    int sockd;
    char id[ID_MAX_LEN];
    bool online;
    vector<Topic> topics;
    vector<MessageTCP> lost_messages;

    ClientTCP() = default;

    ClientTCP(int sockfd, char *id) {
        strcpy(this->id, id);
        this->sockd = sockfd;
        this->online = true;
    }
};

#define DIE(assertion, call_description)                \
    do {                                \
        if (assertion) {                    \
            fprintf(stderr, "(%s, %d): ",            \
                    __FILE__, __LINE__);        \
            perror(call_description);            \
            exit(EXIT_FAILURE);                \
        }                            \
    } while(0)
#endif
