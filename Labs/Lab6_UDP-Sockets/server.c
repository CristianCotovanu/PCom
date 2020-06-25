#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "helpers.h"

void usage(char*file) {
	fprintf(stderr,"Usage: %s server_port file\n",file);
	exit(0);
}

/*
*	Utilizare: ./server server_port nume_fisier
*/
int main(int argc,char**argv) {
	int fd;

	if (argc !=3) {
        usage(argv[0]);
	}

	struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char buff[BUFLEN];

	/*Deschidere socket*/
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
//    memset(&client_addr, 0, sizeof(client_addr));
    memset(buff, 0, BUFLEN);

	/*Setare struct sockaddr_in pentru a asculta pe portul respectiv */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

//    int enable = 1;
//    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
//        perror("setsocketopt");
//        exit(1);
//    }
	
	/* Legare proprietati de socket */
    socklen_t client_len = sizeof(client_addr);
    bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    recvfrom(sockfd, buff, BUFLEN, 0, (struct sockaddr *) &server_addr, &client_len);

	/* Deschidere fisier pentru scriere */
	DIE((fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1, "open file");

	/*
	*  cat_timp  mai_pot_citi
	*		citeste din socket
	*		pune in fisier
	*/
	while(1) {
	    recvfrom(sockfd, buff, BUFLEN, 0,
                 (struct sockaddr *) &server_addr, &client_len);

	    if (strcmp(buff, "READ_OVER") == 0) {
	        break;
	    }

	    write(fd, buff, strlen(buff));
	    memset(buff, 0, BUFLEN);
	}

	/*Inchidere socket*/	
    close(sockfd);
	
	/*Inchidere fisier*/
    close(fd);

	return 0;
}
