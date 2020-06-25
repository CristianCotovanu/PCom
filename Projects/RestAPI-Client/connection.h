#ifndef _HELPERS_
#define _HELPERS_

#define BUFF_LENGTH 4096

// shows the current Error
void Error(const char *msg);

// adds a line to a string message
void ComputeMessage(char *message, const char *line);

// opens a connection with server hostIp on port portno, returns a socket
int OpenConnection(char *hostIp, int portno, int ipType, int socketType, int flag);

// closes a server connection on socket sockfd
void CloseConnection(int sockfd);

// send a message to a server
void SendToServer(int sockfd, char *message);

// receives and returns the message from a server
char *ReceiveFromServer(int sockfd);

#endif
