#ifndef LIB
#define LIB

#define MSGSIZE		1400
#define MSGSIZE_PARITY_FLAG 1396

typedef struct {
  int len;
  char payload[MSGSIZE];
} msg;

typedef struct {
    int parity;
    char payload[MSGSIZE_PARITY_FLAG];
} packet;

void init(char* remote,int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);

#endif

