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

int init_child_socket(struct sockaddr_in *serv_addr){
    // on trouve le port à bind pour l'enfant 
    int port = ntohs(serv_addr->sin_port);
    int socket;
    //Child socket
    while ((socket = init_server_socket(serv_addr, port)) < 0)
    {
        port++;
    }

    return socket;
}

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

int new_client_process(int *nb_client, int max_client){
    if(*nb_client < max_client){
        nb_client++;
        int pid = fork();
        if(pid<0) {
            die("Erreur création enfant");
        }
        return pid;
    }

    return -1;
}

packet get_packet_to_send(packet packet_received, int *audio_fd, int *sample_rate, int *sample_size, int *channels, char *filename, int *filter, int *end){
    packet packet_send;
    char buffer[BUFF_SIZE];
    switch (packet_received.type)
    {
        case FILENAME:

            parseFilenameFilter(packet_received.data, filename, filter);
            // recuperation metadata dans un packet
            *audio_fd = aud_readinit(filename, sample_rate, sample_size, channels);

            if (*audio_fd < 0)
            { // si le serveur ne peut pas ouvrir le fichier demandé
                // envoi d'un message d'erreur
                init_packet(&packet_send, ERROR, "Impossible d'ouvrir le fichier demandé");
            }
            else
            {
                if(*filter == SLOW){
                sprintf(buffer, "%d|%d|%d", *sample_rate/2, *sample_size, *channels);
                }
                else{
                    sprintf(buffer, "%d|%d|%d", *sample_rate, *sample_size, *channels);
                }
                init_packet(&packet_send, HEADER, buffer);
                
            }
            break;

        case RESEND:
            return packet_received;

        case NEXT_BLOCK:
            if (read(*audio_fd, buffer, BUFF_SIZE) != 0){

                apply_filter(buffer, *filter, *sample_size);
                printf("gnegne");
                init_packet(&packet_send,BLOCK, buffer);
                
            } else {
                init_packet(&packet_send,END,"Lecture terminée");
            }
            break;

        case CLOSE_CNX:
            close(*audio_fd);
            init_packet(&packet_send,EMPTY,"");
            *end=1;
            break;
    } //switch

    return packet_send;
}

void parseFilenameFilter(char *data, char *filename, int *filter){
    int i = 0;
    char* a;
    for (a = strtok(data, "|"); a != NULL; a = strtok(NULL, "|")) {
        switch (i)
        {
        case 0: //correspond au rate
            strcpy(filename,a);
            break;
        
        case 1:
            *filter = atoi(a);
            break;
        }
        i++;
    }
}

void apply_filter(char *buffer, int filter, int sample_size){


    printf("apply_filter %d %d \n", filter, sample_size);
    int8_t *ptr8 = (int8_t *)buffer;
    int16_t *ptr16 = (int16_t *)buffer;

    switch(filter){
        case VOLUME:
            if(sample_size == 8){
                for(int i=0; i<BUFF_SIZE; i+= sizeof(int8_t)){
                    *ptr8=(*ptr8)*2;
                    ptr8++;
                }
            }
            else{
                for(int i=0; i<BUFF_SIZE; i+= sizeof(int16_t)){
                    *ptr16=(*ptr16)*2;
                    ptr16++;
                }
            }
            break;
    }
}
