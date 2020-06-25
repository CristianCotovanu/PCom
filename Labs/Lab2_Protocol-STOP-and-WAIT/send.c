#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc,char** argv){
    init(HOST,PORT);
    msg message;
    int file_descriptor, size;

//    Set contains of payload to 0
    memset(message.payload, 0, MAX_LEN);

//    Send name of file
    sprintf(message.payload,"%s", "input.in");
    message.len = strlen(message.payload) + 1;
    send_message(&message);

    // Check response:
    if (recv_message(&message) < 0) {
        perror("Receive error ...");
        return -1;
    } else {
        printf("[send] Got reply with payload: %s\n", message.payload);
    }

//    Send size of file
    file_descriptor = open("input.in", O_RDONLY);
    size = lseek(file_descriptor, 0, SEEK_END);

    sprintf(message.payload, "%d", size);
    message.len = strlen(message.payload) + 1;
    send_message(&message);

    lseek(file_descriptor, 0, SEEK_SET);

    if (recv_message(&message) < 0){
        perror("Receive error ...");
    }
    else {
        printf("[send] Got reply with payload: %s\n", message.payload);
    }

//    Divide file in messages of MAX_LEN - 1
    int crt_msg_size = 0;

    while ((crt_msg_size = read(file_descriptor, message.payload, MAX_LEN - 1))) {
        if (crt_msg_size < 0) {
            perror("Cannot read from input file ...\n");
        } else {
            message.len = crt_msg_size;
            send_message(&message);

            if (recv_message(&message) < 0){
                perror("receive error");
            } else {
                printf("[send] Got reply with payload: ACK(%s)\n", message.payload);
            }

            memset(message.payload, 0, MAX_LEN);
        }
    }

    close(file_descriptor);
    return 0;
}
