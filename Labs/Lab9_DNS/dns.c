#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int usage(char* name) {
	printf("Usage:\n\t%s -n <NAME>\n\t%s -a <IP>\n", name, name);
	return 1;
}

// Receives a name and prints IP addresses
void get_ip(char* name) {
	int ret;
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* p;

	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_UDP;

	// get addresses
	ret = getaddrinfo(name, NULL, &hints, &result);
	if (ret < 0) {
		perror("Error...\n");
	}

	// iterate through addresses and print them
	for (p = result; p != NULL; p = p->ai_next) {
		if (p->ai_family == AF_INET) {
			char ip[INET_ADDRSTRLEN];
			struct sockaddr_in* sockaddr = (struct sockaddr_in* )p->ai_addr;
			if (inet_ntop(p->ai_family, &(sockaddr->sin_addr), ip, sizeof(ip))) {
				printf("IPv4 addr: %s\n", ip);
            }
		} else if (p->ai_family == AF_INET6) {
		    char ip[INET6_ADDRSTRLEN];
		    struct sockaddr_in6 *sockaddrIn6 = (struct sockaddr_in6 *)p->ai_addr;
		    if (inet_ntop(p->ai_family, &(sockaddrIn6->sin6_addr), ip, sizeof(ip))) {
		        printf("IPv6 addr: %s\n", ip);
		    }
		}
	}
	// free allocated data
	freeaddrinfo(result);
}

// Receives an address and prints the associated name and service
void get_name(char* ip) {
	int ret;
	struct sockaddr_in addr;
	char host[1024];
	char service[20];

	// fill in address data
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    inet_aton(ip, &addr.sin_addr);

	// get name and service
    ret = getnameinfo((struct sockaddr* ) &addr, sizeof(struct sockaddr_in), host, 1024, service, 20, 0);
    if (ret < 0) {
        perror("Error...\n");
    }
	// print name and service
	printf("Hostname is: %s\n", host);
    printf("Service is: %s\n", service);
}

int main(int argc, char **argv) {
	if (argc < 3) {
		return usage(argv[0]);
	}

	if (strncmp(argv[1], "-n", 2) == 0) {
		get_ip(argv[2]);
	} else if (strncmp(argv[1], "-a", 2) == 0) {
		get_name(argv[2]);
	} else {
		return usage(argv[0]);
	}

	return 0;
}
