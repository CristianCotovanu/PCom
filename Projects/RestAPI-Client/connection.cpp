#include <cstdlib>     /* exit, atoi, malloc, free */
#include <cstdio>
#include <unistd.h>     /* read, write, close */
#include <cstring>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <arpa/inet.h>
#include "connection.h"
#include "buffer.h"

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void Error(const char *msg) {
    perror(msg);
    exit(0);
}

void ComputeMessage(char *message, const char *line) {
    strcat(message, line);
    strcat(message, "\r\n");
}

int OpenConnection(char *hostIp, int portno, int ipType, int socketType, int flag) {
    sockaddr_in servAddr{};
    int sockfd = socket(ipType, socketType, flag);
    if (sockfd < 0)
        Error("ERROR opening socket");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = ipType;
    servAddr.sin_port = htons(portno);
    inet_aton(hostIp, &servAddr.sin_addr);

    if (connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        Error("ERROR connecting");

    return sockfd;
}

void CloseConnection(int sockfd) {
    close(sockfd);
}

void SendToServer(int sockfd, char *message) {
    int bytes, sent = 0;
    int total = strlen(message);

    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            Error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *ReceiveFromServer(int sockfd) {
    char response[BUFF_LENGTH];
    buffer buffer = BufferInit();
    int headerEnd = 0;
    int contentLength = 0;

    do {
        int bytes = read(sockfd, response, BUFF_LENGTH);

        if (bytes < 0) {
            Error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        BufferAdd(&buffer, response, (size_t) bytes);

        headerEnd = BufferFind(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (headerEnd >= 0) {
            headerEnd += HEADER_TERMINATOR_SIZE;

            int contentLengthStart = BufferFindInsensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);

            if (contentLengthStart < 0) {
                continue;
            }

            contentLengthStart += CONTENT_LENGTH_SIZE;
            contentLength = strtol(buffer.data + contentLengthStart, nullptr, 10);
            break;
        }
    } while (true);

    size_t total = contentLength + (size_t) headerEnd;

    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFF_LENGTH);
        if (bytes < 0) {
            Error("ERROR reading response from socket");
        }
        if (bytes == 0) {
            break;
        }
        BufferAdd(&buffer, response, (size_t) bytes);
    }
    BufferAdd(&buffer, "", 1);
    return buffer.data;
}
