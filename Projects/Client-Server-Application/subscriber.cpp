#include "subscriber.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        usage_subscriber();
    }

    int sockfd;
    int ret;
    int timeout = 100000;
    sockaddr_in server_addr{};

    int fdmax;
    fd_set read_fds;
    fd_set tmp_fds;

    /// Golire cele doua multimi de descriptori pt TCP
    FD_ZERO(&tmp_fds);
    FD_ZERO(&read_fds);
    /// Deschidere socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");
    /// Setare adresa server
    int port = atoi(argv[3]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    ret = inet_aton(argv[2], &server_addr.sin_addr);
    DIE(ret == 0, "inet_aton");
    /// Stabilire conexiune cu serverul
    ret = connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    DIE(ret < 0, "connect");
    /// Adaugare socket client in multime
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;
    FD_SET(0, &read_fds);

    /// Trimiterea ID-ului pentru verificarea sa in server
    char temp_buff[ID_MAX_LEN];
    strcpy(temp_buff, argv[1]);
    temp_buff[strlen(argv[1]) + 1] = '\0';

    ret = send(sockfd, temp_buff, strlen(temp_buff), 0);
    DIE(ret < 0, "ID sending failed");

    memset(temp_buff, 0, ID_MAX_LEN);

    /// Opreste Algoritmul lui Nagle
    int enable = 1;
    ret = setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &enable, sizeof(enable));
    DIE(ret == -1, "setsocketopt");

    timeval t{};
    t.tv_usec = 1000;
    t.tv_sec = 100;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &t, sizeof(struct timeval));

    while (timeout != 0) {
        tmp_fds = read_fds;

        ret = select(fdmax + 1, &tmp_fds, nullptr, nullptr, nullptr);
        DIE(ret < 0, "Client closes.");

        if (FD_ISSET(0, &tmp_fds)) {
            char input[TCP_CLIENT_INPUT_LEN];
            memset(input, 0, TCP_CLIENT_INPUT_LEN);
            /// Se citeste de la tastatura
            fgets(input, TCP_CLIENT_INPUT_LEN, stdin);

            char tmp[TCP_CLIENT_INPUT_LEN];
            strcpy(tmp, input);
            char *input_token = strtok(tmp, " ");

            if (strcmp(input_token, "subscribe") == 0) {
                input_token = strtok(nullptr, " ");
                if (input_token != nullptr) {
                    char topic[TOPIC_MAX_LEN];
                    strcpy(topic, input_token);
                    input_token = strtok(nullptr, " \n");

                    if (input_token != nullptr) {
                        ret = send(sockfd, input, strlen(input), 0);
                        DIE(ret < 0, "send");

                        MessageTCP recv_tcp_msg{};
                        ret = recv(sockfd, &recv_tcp_msg, sizeof(MessageTCP), 0);
                        DIE(ret < 0, "recv");

                        if (strcmp(recv_tcp_msg.data_body, "Failure") == 0) {
                            cout << "You are already subscribed to " << topic << "." << endl;
                        } else {
                            ret = send(sockfd, input, strlen(input), 0);
                            DIE(ret < 0, "send");
                            cout << "Subscribed " << topic << "." << endl;
                        }
                    } else {
                        cout << "Wrong arguments for a subscribe." << endl;
                    }
                } else {
                    cout << "Wrong arguments for a subscribe." << endl;
                }
            } else if (strcmp(input_token, "unsubscribe") == 0) {
                input_token = strtok(nullptr, " \n");
                if (input_token != nullptr) {
                    ret = send(sockfd, input, strlen(input), 0);
                    DIE(ret < 0, "send");

                    MessageTCP recv_tcp_msg{};
                    ret = recv(sockfd, &recv_tcp_msg, sizeof(MessageTCP), 0);
                    DIE(ret < 0, "recv");

                    if (strcmp(recv_tcp_msg.data_body, "Failure") == 0) {
                        cout << "You are not subscribed to " << input_token << "." << endl;
                    } else {
                        cout << "Unsubscribed " << input_token << "." << endl;
                    }
                } else {
                    cout << "Wrong arguments for an unsubscribe." << endl;
                }
            } else if (strcmp(input_token, "exit\n") == 0) {
                ret = shutdown(sockfd, SHUT_RDWR);
                DIE(ret < 0, "Can't exit");
                break;
            } else {
                cout << "Invalid command." << endl;
            }
        } else {
            MessageTCP recv_tcp_msg{};
            recv(sockfd, &recv_tcp_msg, sizeof(MessageTCP), 0);
            if (recv_tcp_msg.port == CLOSE_CLIENT) {
                /// S-a primit un mesaj pentru a face clientul sa se inchida
                cout << recv_tcp_msg.data_body << endl;
                ret = shutdown(sockfd, SHUT_RDWR);
                DIE(ret < 0, "Nu se poate da exit\n");
                break;
            } else if (recv_tcp_msg.port != DUMMY_FLAG) {
                if (recv_tcp_msg.port == 0) {
                    --timeout;
                }
                /// S-a primit update de la un client UDP redirectionat de catre server
                print_tcp_message(recv_tcp_msg);
            }
        }
    }

    close(sockfd);
    return 0;
}
