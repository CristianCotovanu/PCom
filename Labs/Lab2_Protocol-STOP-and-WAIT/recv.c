#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc,char** argv){
    msg received, msg_to_send;
    init(HOST,PORT);

    if (recv_message(&received) < 0){
        perror("Receive message");
        return -1;
    }
    printf("[recv] Got msg with payload: <%s>, sending ACK...\n", received.payload);

//    Create file with the same name as before + "_out", ultimul argument reprezinta permisiunile fisierului
    strcat(received.payload ,"_out");
    int rec_file_descriptor = open(received.payload, O_WRONLY | O_CREAT, 0777);

    sprintf(msg_to_send.payload, "ACK(%s)", received.payload);
    msg_to_send.len = strlen(msg_to_send.payload) + 1;
    send_message(&msg_to_send);

    if (recv_message(&received) < 0) {
        perror("Receive message ...");
        return -1;
    }
    printf("[recv] Got msg with payload: %s\n", received.payload);

    sprintf(msg_to_send.payload, "ACK(%s)", received.payload);
    msg_to_send.len = strlen(msg_to_send.payload);
    send_message(&msg_to_send);


    int remaining_size = atoi(received.payload);
//    int crt_msg_size;
    printf("Input file size: %d \n", remaining_size);

    while (remaining_size) {
        if (recv_message(&received) < 0) {
            perror("Receive message ...");
            return -1;
        }

        printf("[recv] Got msg with payload: %s\n", received.payload);

//        crt_msg_size = write(rec_file_descriptor, received.payload, received.len);

        send_message(&received);
        remaining_size -= received.len;

        memset(msg_to_send.payload, 0, MAX_LEN);
    }

    close(rec_file_descriptor);
    return 0;
}