#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "helpers.h"

void run_client(int sockfd) {
    char buff[BUFLEN];
    memset(buff, 0, BUFLEN);

    int buff_len;

    while (read(STDIN_FILENO, buff, BUFLEN - 1) > 0  && !isspace(buff[0])) {
        buff_len = strlen(buff) + 1;

        // TODO 4: Trimiteti mesajul catre server prin sockfd
        send(sockfd, buff, buff_len, 0);
        memset(buff, 0, BUFLEN);

        // TODO 5: Receptionati un mesaj venit de la server
        int bytes_received = recv(sockfd, buff, buff_len, 0);

        if(bytes_received < 0) {
            fprintf(stderr, "Receive error..");
        }
        
        fprintf(stderr, "[Sent]: %s", buff);

        memset(buff, 0, BUFLEN);
    }
}

void run_two_clients(int sockfd) {
	char buff[BUFLEN];
    memset(buff, 0, BUFLEN);

    recv(sockfd, buff, 2, 0);
    int client_nr = buff[0] == '1' ? 1 : 2;
    memset(buff, 0, BUFLEN);

    int bytes_remaining = 0;
    int bytes_received = 0;
    int buff_len = 0;
    int bytes_sent = 0;

    while (1) {
        if (client_nr == 1) {
            read(STDIN_FILENO, buff, BUFLEN - 1);
            buff_len = strlen(buff) + 1;
            
            bytes_sent = send(sockfd, buff, buff_len, 0);
            bytes_remaining = buff_len - bytes_sent;

            while (bytes_remaining != 0) {
                bytes_sent = send(sockfd, &buff[bytes_sent], bytes_remaining, 0);
                bytes_remaining -= bytes_sent;
            }

            memset(buff, 0, BUFLEN);
            bytes_received = recv(sockfd, buff, BUFLEN, 0);
            
            if (bytes_received < 0) {
                fprintf(stderr, "Receive error..");
                break;
            }

            fprintf(stderr, "[Client2]: %s", buff);
        } else {
            bytes_received = recv(sockfd, buff, BUFLEN, 0);
            
            if (bytes_received < 0) {
                fprintf(stderr, "Receive error..");
                break;
            }

            fprintf(stderr, "[Client1]: %s", buff);
            memset(buff, 0, BUFLEN);

            read(STDIN_FILENO, buff, BUFLEN - 1);
            buff_len = strlen(buff) + 1;
            
            bytes_sent = send(sockfd, buff, buff_len, 0);
            bytes_remaining = buff_len - bytes_sent;

            while (bytes_remaining != 0) {
                bytes_sent = send(sockfd, &buff[bytes_sent], bytes_remaining, 0);
                bytes_remaining -= bytes_sent;
            }
        }
        memset(buff, 0, BUFLEN);
    }
}


int main(int argc, char* argv[])
{
    int sockfd = -1;
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);
    memset(&serv_addr, 0, socket_len);
    char buff[BUFLEN];

    if (argc != 3) {
        printf("\n Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    // TODO 1: Creati un socket TCP pentru conectarea la server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // TODO 2: Completati in serv_addr adresa serverului, familia de adrese si portul pentru conectare
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
//    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    inet_aton(argv[1], &(serv_addr.sin_addr));

    // TODO 3: Creati conexiunea catre server
    connect(sockfd, (struct sockaddr *) &serv_addr, socket_len);

    run_client(sockfd);
//    run_two_clients(sockfd);

    // TODO 6: Inchideti conexiunea si socketul creat
    close(sockfd);

    return 0;
}
