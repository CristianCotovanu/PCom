#include "server.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        usage_server();
    }

    int ret;
    char buff[BUFF_LEN];
    memset(buff, 0, BUFF_LEN);

    sockaddr_in udp_server_addr{};
    sockaddr_in udp_client_addr{};
    socklen_t udp_client_len;

    sockaddr_in tcp_server_addr{};
    sockaddr_in tcp_client_addr{};
    socklen_t tcp_client_len;

    int new_client_sockfd;
    int fdmax;
    fd_set read_fds;
    fd_set tmpFds;

    vector<ClientTCP> tcp_clients;

    /// Golire cele doua multimi de descriptori pt TCP
    FD_ZERO(&read_fds);
    FD_ZERO(&tmpFds);
    /// Deschidere socket TCP si UDP
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(udp_sockfd < 0, "socket_udp");
    DIE(tcp_sockfd < 0, "socket_tcp");
    /// Portul se gaseste in argv[1]
    int port = atoi(argv[1]);
    DIE(port == 0, "atoi");

    /// Setare adresa server UDP
    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(port);
    udp_server_addr.sin_addr.s_addr = INADDR_ANY;

    /// Setare adresa server TCP
    memset(&tcp_server_addr, 0, sizeof(tcp_server_addr));
    tcp_server_addr.sin_family = AF_INET;
    tcp_server_addr.sin_port = htons(port);
    tcp_server_addr.sin_addr.s_addr = INADDR_ANY;

    /// Setare socket UDP
    udp_client_len = sizeof(udp_client_addr);
    ret = bind(udp_sockfd, (struct sockaddr *) &udp_server_addr, sizeof(udp_server_addr));
    DIE(ret < 0, "bind");

    /// Setare socket TCP
    ret = bind(tcp_sockfd, (struct sockaddr *) &tcp_server_addr, sizeof(tcp_server_addr));
    DIE(ret < 0, "bind");
    ret = listen(tcp_sockfd, MAX_CLIENTS);
    DIE(ret < 0, "listen");

    /// Adauga in fdset socket-ul pe care se asculta conexiuni
    FD_SET(0, &read_fds);
    FD_SET(udp_sockfd, &read_fds);
    FD_SET(tcp_sockfd, &read_fds);
    fdmax = tcp_sockfd;

    /// Opreste Algoritmul lui Nagle
    int enable = 1;
    ret = setsockopt(tcp_sockfd, SOL_TCP, TCP_NODELAY, &enable, sizeof(enable));
    DIE(ret == -1, "setsocketopt");

    while (true) {
        tmpFds = read_fds;

        /// tmpFds contine doar socketi pe care s-au primit date/conexiuni dupa SELECT
        ret = select(fdmax + 1, &tmpFds, nullptr, nullptr, nullptr);
        if (ret < 0) {
            /// Serverul se inchide
            cout << "Server shutting down..." << endl;
            break;
        }

        for (int sock = 1; sock <= fdmax; sock++) {
            if (FD_ISSET(sock, &tmpFds)) {
                /// A venit o cerere de conexiune pe socketul inactiv (cel cu listen)
                /// pe care serverul o accepta
                if (sock == tcp_sockfd) {
                    tcp_client_len = sizeof(tcp_client_addr);
                    new_client_sockfd = accept(tcp_sockfd,
                                               (struct sockaddr *) &tcp_client_addr, &tcp_client_len);
                    DIE(new_client_sockfd < 0, "accept");

                    /// Se adauga noul socket intors de accept() la multimea descriptorilor de citire
                    FD_SET(new_client_sockfd, &read_fds);

                    char new_client_id[ID_MAX_LEN];
                    memset(new_client_id, 0, ID_MAX_LEN);
                    ret = recv(new_client_sockfd, new_client_id, sizeof(new_client_id), 0);
                    DIE(ret < 0, "recv");

                    OfflineClient new_client = find_offline_client(tcp_clients, new_client_id);

                    if (new_client.exists && !new_client.online) {
                        /// Clientul deja exista pe server dar a fost deconectat
                        for (int c = 0; c < tcp_clients.size(); c++) {
                            if (strcmp(tcp_clients[c].id, new_client.client.id) == 0) {
                                tcp_clients[c].online = true;
                                tcp_clients[c].sockd = new_client_sockfd;
                                cout << "Existing client " << tcp_clients[c].id << " connected from "
                                     << inet_ntoa(tcp_client_addr.sin_addr) << ":"
                                     << ntohs(tcp_client_addr.sin_port) << endl;


                                for (int m = 0; m < tcp_clients[c].lost_messages.size(); m++) {
                                    send(tcp_clients[c].sockd, &tcp_clients[c].lost_messages[m],
                                         sizeof(MessageTCP), 0);
                                }
                                tcp_clients[c].lost_messages.clear();
                                break;
                            }
                        }
                    } else if (new_client.exists && new_client.online) {
                        /// Clientul deja exista pe server si este online. Conexiune refuzata

                        char message[LONG_MSG_LEN] = "Client ID already exists.\0";
                        send_error_message(new_client_sockfd, CLOSE_CLIENT, message);
                        /// Se scoate socket-ul creat pentru ca nu va fi folosit.
                        close(new_client_sockfd);
                        FD_CLR(new_client_sockfd, &read_fds);
                    } else {
                        if (new_client_sockfd > fdmax) {
                            fdmax = new_client_sockfd;
                        }

                        /// Adaugare client nou si afisare la consola
                        ClientTCP new_client(new_client_sockfd, new_client_id);
                        tcp_clients.push_back(new_client);

                        cout << "New client " << new_client_id << " connected from "
                             << inet_ntoa(tcp_client_addr.sin_addr) << ":"
                             << ntohs(tcp_client_addr.sin_port) << endl;
                    }
                } else if (sock == udp_sockfd) {
                    /// Clientul de pe care s-au primit date este UDP
                    MessageUDP recv_udp_msg{};

                    recvfrom(udp_sockfd, &recv_udp_msg, sizeof(MessageUDP), 0,
                             (struct sockaddr *) &udp_server_addr, &udp_client_len);

                    MessageTCP send_tcp_msg = udp_to_tcp(recv_udp_msg);
                    send_tcp_msg.port = udp_server_addr.sin_port;
                    strcpy(send_tcp_msg.ip_addr, inet_ntoa(udp_server_addr.sin_addr));

                    for (int c = 0; c < tcp_clients.size(); c++) {
                        for (int t = 0; t < tcp_clients[c].topics.size(); t++) {
                            if (strcmp(tcp_clients[c].topics[t].name, send_tcp_msg.topic) == 0) {
                                if (tcp_clients[c].online && udp_sockfd != tcp_clients[c].sockd) {
                                    send(tcp_clients[c].sockd, &send_tcp_msg,
                                         sizeof(MessageTCP), 0);
                                } else if (tcp_clients[c].topics[t].sf == 1) {
                                    tcp_clients[c].lost_messages.push_back(send_tcp_msg);
                                }
                            }
                        }
                    }
                } else {
                    /// S-au primit date pe unul din socketii de client TCP,
                    /// asa ca serverul trebuie sa le receptioneze
                    memset(buff, 0, BUFF_LEN);
                    ret = recv(sock, buff, BUFF_LEN, 0);
                    DIE(ret < 0, "recv");

                    if (ret != 0) {
                        /// Comanda de la client TCP
                        char *recv_token;
                        recv_token = strtok(buff, " ");
                        if (strcmp(recv_token, "subscribe") == 0) {
                            ClientTCP *requester = nullptr;
                            Topic new_topic{};
                            bool already_subbed = false;

                            for (int c = 0; c < tcp_clients.size(); c++) {
                                if (tcp_clients[c].sockd == sock) {
                                    requester = &tcp_clients[c];
                                    break;
                                }
                            }

                            if (requester != nullptr) {
                                recv_token = strtok(nullptr, " ");
                                DIE(recv_token == nullptr, "Topic parameter missing");
                                for (int t = 0; t < requester->topics.size(); t++) {
                                    if (strcmp(recv_token, requester->topics[t].name) == 0) {
                                        already_subbed = true;
                                    }
                                }

                                if (!already_subbed) {
                                    strcpy(new_topic.name, recv_token);
                                    recv_token = strtok(nullptr, " \n");     // Move to sf
                                    if (recv_token != nullptr) {
                                        new_topic.sf = atoi(recv_token) == 1 ? 1 : 0;
                                        requester->topics.push_back(new_topic);

                                        char msg[DUMMY_MSG_LEN] = "Succes";
                                        send_error_message(requester->sockd, DUMMY_FLAG, msg);
                                    }
                                } else {
                                    char msg[DUMMY_MSG_LEN] = "Failure";
                                    send_error_message(requester->sockd, DUMMY_FLAG, msg);
                                }
                            } else {
                                cout << "No client found." << endl;
                            }
                        } else if (strcmp(recv_token, "unsubscribe") == 0) {
                            recv_token = strtok(nullptr, " \n");

                            for (int c = 0; c < tcp_clients.size(); c++) {
                                int remove_idx = -1;
                                for (int t = 0; t < tcp_clients[c].topics.size(); t++) {
                                    if (strcmp(tcp_clients[c].topics[t].name, recv_token) == 0) {
                                        remove_idx = t;
                                        break;
                                    }
                                }
                                if (remove_idx != -1) {
                                    tcp_clients[c].topics.erase(tcp_clients[c].topics.begin() + remove_idx);

                                    char msg[DUMMY_MSG_LEN] = "Succes";
                                    send_error_message(tcp_clients[c].sockd, DUMMY_FLAG, msg);
                                } else {
                                    char msg[DUMMY_MSG_LEN] = "Failure";
                                    send_error_message(tcp_clients[c].sockd, DUMMY_FLAG, msg);
                                }
                            }
                        } else if (strcmp(recv_token, "exit") == 0) {
                            close_client(sock, read_fds, tcp_clients);
                        }
                    } else {
                        /// Clientul "sock" a inchis conexiunea.
                        /// Printeaza la stdout si scoate din multimea de citire socketul clientului inchis
                        close_client(sock, read_fds, tcp_clients);
                    }
                }
            } else if (FD_ISSET(0, &tmpFds)) {
                char input[SERVER_MAX_INPUT_LEN];
                memset(input, 0, SERVER_MAX_INPUT_LEN);
                /// Se citeste de la tastatura
                fgets(input, SERVER_MAX_INPUT_LEN, stdin);
                char *input_token = strtok(input, " \n");

                if (input_token != nullptr && strcmp(input_token, "exit") == 0) {
                    char message[LONG_MSG_LEN] = "\0";
                    for (int sck = 5; sck <= fdmax; sck++) {
                        send_error_message(sck, CLOSE_CLIENT, message);
                    }

                    for (int sck = 5; sck <= fdmax; sck++) {
                        close(sck);
                        FD_CLR(sck, &read_fds);
                    }

                    close(tcp_sockfd);
                    close(udp_sockfd);
                    break;
                } else if (input_token != nullptr && strcmp(input_token, "exit") != 0) {
                    cout << "Invalid command." << endl;
                }
            }
        }
    }

    close(tcp_sockfd);
    close(udp_sockfd);
    return 0;
}