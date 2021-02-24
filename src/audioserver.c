#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../include/clientserver.h"
#include "../include/packet.h"
#include "../include/clientserver.h"
#include "../include/lib/sysprog-audio-1.5/audio.h"

#define RED   "\x1B[31m"
#define GRE   "\x1b[32m"
#define RESET "\x1B[0m"


int main(){
    printf("# "GRE"Lancement du serveur\n"RESET"#\n");

    /* ##### VARIABLES ##### */
    // Network
    struct sockaddr_in serv_addr, client_addr;
    packet packet_send;
    packet packet_received;   
    socklen_t len;
    socklen_t flen = sizeof(struct sockaddr_in);
    int socketfd;
    
    // Audio
    int sample_rate, sample_size, channels;
    int audio_file;
    char buffer[BUFF_SIZE];
    int send = 0;
    int get = 0;
    int longueur = 0;
    
    /* #### NETWORK INITIALISATION #### */
    socketfd = init_server_socket(&serv_addr,1234);
    if(socketfd<0)
        die("erreur socket");

    
    /* #### DEBUT COMMUNICATION #### */
    // le serveur ne s'arrête que sur erreur
    while(1){
        //  reset des paquet à envoyer
        clear_packet(&packet_send);
        clear_packet(&packet_received);

        // recepetion d'un nouveau paquet
        len  = recvfrom(socketfd, &packet_received, sizeof(packet), 0, (struct sockaddr*) &client_addr, &flen);
            if(len < 0) die("Erreur lors de la réception");
        printf("code packet : %d\n", packet_received.type);
        get++;

        switch (packet_received.type)
        {
            case FILENAME:
                // recuperation metadata
                audio_file = aud_readinit(packet_received.data, &sample_rate, &sample_size, &channels);
                
                if(audio_file < 0){ // si le serveur ne peut pas ouvrir le fichier demandé
                    // envoi d'un message d'erreur
                    init_packet(&packet_send, ERROR, "Impossible d'ouvrir le fichier demandé");
                } else {
                    sprintf(buffer, "%d|%d|%d", sample_rate, sample_size, channels);
                    init_packet(&packet_send, HEADER, buffer);
                }
                break;

            case RESEND:
            printf(GRE"Renvoi du paquet\n"RESET);
                init_packet(&packet_send,BLOCK, buffer);
            break;
            
            case NEXT_BLOCK:
                
				if (read(audio_file, buffer, BUFF_SIZE) != 0){
                    longueur += strlen(buffer);
                    init_packet(&packet_send,BLOCK, buffer);

                } else {
                    init_packet(&packet_send,END,"Lecture terminée");
                }
			    break;

            case CLOSE_CNX:
                close(audio_file);

                // verifications receptions
                printf("Lecture terminée\n");
                printf("Paquet envoyés : %d\n",send);
                printf("Paquet reçus : %d\n",get);
                printf("Taille : %d\n",longueur);

                send = 0;
                get = 0;
                break;
        }
        if(send != 54){
            if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr*) &client_addr, flen) < 0)
            die("Erreur lors de l'envoie\n");
        } else{
            printf(RED"paquet non envoyé\n"RESET);
        }
        
        send++;
    }

    close(socketfd);

    return 0;
}