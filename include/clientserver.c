#include <string.h>
#include "../include/clientserver.h"

struct in_addr* resolve_host(char *hostname){
    struct hostent *resolve;
    resolve = gethostbyname(hostname);
    if (resolve==NULL) {
        die("Cannot resolve hostname");
    }

    return (struct in_addr*) resolve->h_addr_list[0];
}

int init_server_socket (struct sockaddr_in *server_addr, uint16_t listen_port){
    int sockfd;

    server_addr->sin_family      = AF_INET;
    server_addr->sin_port        = htons(listen_port);
    server_addr->sin_addr.s_addr = htonl(INADDR_ANY);  
    
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd < 0) return -1;

    if( bind(sockfd, (struct sockaddr*) server_addr, sizeof(struct sockaddr_in)) < 0)
        return -1;
    
    return sockfd;
}

int init_client_socket (struct sockaddr_in *serv_addr,  uint16_t server_port, char *hostname){

    struct in_addr* server_ip = resolve_host(hostname);
    serv_addr->sin_family      = AF_INET;
    serv_addr->sin_port        = htons(server_port);
    serv_addr->sin_addr        = *server_ip;
    
    int socketfd = socket(AF_INET,SOCK_DGRAM,0);
    if(socketfd < 0) return -1;
    
    return socketfd;
}

int send_packet(packet *packet);

int get_packet(packet *packet);

void die(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}

void parseMetadata(char *buffer, int *sample_rate, int *sample_size, int *channels) {
    int i = 0;
    char* a;
    for (a = strtok(buffer, "|"); a != NULL; a = strtok(NULL, "|")) {
        switch (i)
        {
        case 0: //correspond au rate
            *sample_rate = atoi(a);
            break;
        
        case 1:
            *sample_size = atoi(a);
            break;
        
        case 2:
            *channels = atoi(a);
            break;
        }
        i++;
    }
}