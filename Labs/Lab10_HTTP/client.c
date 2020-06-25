#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

int main(int argc, char *argv[])
{
    char *addr;
    char *message;
    int sockfd;

    // Ex 1.1: GET dummy from main server
    printf("--------------------GET--------------------\n");
    message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/dummy", NULL, NULL, 0);
    addr = inet_ntoa((struct in_addr)*((struct in_addr *) gethostbyname("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com")->h_addr_list[0]));
    sockfd = open_connection(addr, 8080, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);

    printf("[GET REQUEST] %s\n", message);
    printf("[REPLY] %s\n\n\n", receive_from_server(sockfd));
    close_connection(sockfd);

    // Ex 1.2: POST dummy and print response from main server
    printf("--------------------POST--------------------\n");
    char **body_content = (char **)malloc(sizeof(char*));
    body_content[0] = (char *)malloc(100 * sizeof(char));

    message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/dummy", "application/x-www-form-urlencoded", (char**) body_content, 1, NULL, 0);
    addr = inet_ntoa((struct in_addr)*((struct in_addr *) gethostbyname("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com")->h_addr_list[0]));
    sockfd = open_connection(addr, 8080, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);

    printf("[POST REQUEST] %s\n", message);
    printf("[REPLY] %s\n\n", receive_from_server(sockfd));
    close_connection(sockfd);

    // Ex 2: Login into main server
    printf("--------------------Login Attempt--------------------\n");
    body_content[0] = (char *)calloc(100, sizeof(char));
    body_content[1] = (char *)calloc(100, sizeof(char));
    
    strcpy(body_content[0], "username=student&");
    strcpy(body_content[1], "password=student");

    message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/auth/login", "application/x-www-form-urlencoded", (char**) body_content, 2, NULL, 0);
    addr = inet_ntoa((struct in_addr)*((struct in_addr *) gethostbyname("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com")->h_addr_list[0]));
    sockfd = open_connection(addr, 8080, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);

    printf("[POST REQUEST] %s\n", message);
    printf("[REPLY] %s\n\n", receive_from_server(sockfd));
    free(body_content[0]);
    free(body_content[1]);
    free(body_content);

    // Ex 3: GET weather key from main server
    printf("--------------------GET weather key--------------------\n");
    char **cookies = malloc(sizeof(char*));
    cookies[0] =  malloc(100 * sizeof(char));
    
    strcpy(cookies[0],"connect.sid = s%3AMGNjBsOnqqNuhH2bXV2RMp2ljt1Y0vs8.vWFRUpgZyE1eGuoVUGPhGoQMCLj4iVsBc%2BLLiMMGHjs; Path=/; HttpOnly");
    message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/weather/key", NULL, cookies, 1);
    addr = inet_ntoa((struct in_addr)*((struct in_addr *) gethostbyname("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com")->h_addr_list[0]));
    send_to_server(sockfd, message);
    
    printf("[GET REQUEST] %s\n", message);
    printf("[REPLY] %s\n", receive_from_server(sockfd));

    // Ex 4: GET weather data from OpenWeather API
    // Ex 5: POST weather data for verification to main server
    // Ex 6: Logout from main server
    printf("\n\n--------------------Logout Attempt--------------------\n");
    message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/auth/logout", NULL, NULL, 0);
    addr = inet_ntoa((struct in_addr)*((struct in_addr *) gethostbyname("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com")->h_addr_list[0]));
    send_to_server(sockfd, message);
    
    printf("[GET REQUEST] %s\n", message);
    printf("[REPLY] %s", receive_from_server(sockfd));
    close_connection(sockfd);

    // BONUS: make the main server return "Already logged in!"

    // free the allocated data at the end!    
    

    return 0;
}
