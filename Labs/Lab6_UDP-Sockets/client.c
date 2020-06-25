#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "helpers.h"

void usage(char*file) {
	fprintf(stderr,"Usage: %s ip_server port_server file\n",file);
	exit(0);
}

/*
*	Utilizare: ./client ip_server port_server nume_fisier_trimis
*/
int main(int argc,char**argv) {
	if (argc != 4)
		usage(argv[0]);
	
	int fd, to_read;
	struct sockaddr_in to_station;
	char buff[BUFLEN];

	/*Deschidere socket*/
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&to_station, 0, sizeof(to_station));
    memset(buff, 0, BUFLEN);

    to_station.sin_family = AF_INET;
    to_station.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &to_station.sin_addr);

    /* Deschidere fisier pentru citire */
	DIE((fd = open(argv[3], O_RDONLY)) == -1,"open file");
	
	/*Setare struct sockaddr_in pentru a specifica unde trimit datele*/
	sprintf(buff, "%s", argv[3]);
	sendto(sockfd, buff, strlen(buff), 0,
	        (struct sockaddr *) &to_station, sizeof(to_station));
	memset(buff, 0, BUFLEN);

	/*
	*  cat_timp mai_pot_citi
	*		citeste din fisier
	*		trimite pe socket
	*/
	while ((to_read = read(fd, buff, (BUFLEN - 1)))) {
        sendto(sockfd, buff, to_read, 0, (struct sockaddr *) &to_station, sizeof(to_station));
        memset(buff, 0, BUFLEN);
    }

    memset(buff, 0, BUFLEN);
	sprintf(buff, "READ_OVER");
    sendto(sockfd, buff, strlen("READ_OVER"), 0, (struct sockaddr *) &to_station, sizeof(to_station));

	/*Inchidere socket*/
    close(sockfd);
	
	/*Inchidere fisier*/
    close(fd);

	return 0;
}
