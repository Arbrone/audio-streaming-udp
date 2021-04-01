/**
 * Projet SYS, rendu du 26 mars
 * Binome : Thomas VERRIER, Alex GILLES
 * Groupe : 1.1 
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include "../include/clientserver.h"
#include "../include/packet.h"
#include "../include/lib/sysprog-audio-1.5/audio.h"

#define RED   "\x1B[31m"
#define GRE   "\x1b[32m"
#define RESET "\x1B[0m"

#define TIMEOUT_VALUE 500000


int main(int argc,char *argv[]){
    
    // if(argc != 3) { 
    //     die("Respecter le format : ./client [server_host_name]0 [file_name]");
    // }

    /* #### VARIABLES ####*/
    // Network
    struct sockaddr_in serv_addr;
    packet packet_send;
    packet packet_received;
    socklen_t flen = sizeof(struct sockaddr_in);
    int socketfd;
    

    //Audio
    int sample_rate, sample_size, channels;
    int audio_output;
    int end = 0;

    //Timeout
    int nb;
    fd_set read_set;
    struct timeval timeout;


    /* #### INITIALISATION ####*/
    socketfd = init_client_socket(&serv_addr,1234,argv[1]);
        if(socketfd<0)
            die("erreur socket");
    
    
    /* #### DEBUT COMMUNICATION #### */
    printf("# "GRE"Connexion au serveur\n"RESET"#\n");
    
    // le premier paquet contient uniquement le nom du fichier à lire    
    init_packet(&packet_send, FILENAME, argv[2]);
    sprintf(packet_send.data, "%s|%s", argv[2], argv[3]);
    if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr*) &serv_addr, flen) < 0)
    {
        die("Erreur lors de l'envoie");
    }
 
    // début échange
    while(!end){

        FD_ZERO(&read_set);
        FD_SET(socketfd, &read_set);
        timeout.tv_sec  = 0;
        timeout.tv_usec = TIMEOUT_VALUE;

        nb = select(socketfd+1,&read_set,NULL,NULL,&timeout);

        if(nb<0) die("erreur sur select()");
        
        // Si le timeout arrive a expiration 
        if(nb==0){
            printf(RED"Expiration timeout\n"RESET);
            init_packet(&packet_send,RESEND,"timeout");
            // on redemande le meme packet
            if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr*) &serv_addr, flen) < 0)
                die("Erreur lors de l'envoie");
            printf(GRE"Demande resend\n"RESET);
        }

        if(FD_ISSET(socketfd, &read_set)){
            // remise à 0 des paquets à envoyer
            clear_packet(&packet_send);
            clear_packet(&packet_received);

            if(recvfrom(socketfd, &packet_received, sizeof(packet), 0, (struct sockaddr*) &serv_addr, &flen) < 0)
                die("Erreur lors de la reception");

            // Traitement du paquet reçu 
            switch (packet_received.type)
            {
                case HEADER:
                    if(socketfd<0)
                        die("erreur socket");

                    // récupération des metadata
                    parseMetadata(packet_received.data, &sample_rate, &sample_size, &channels);

                    // et initialisation du lecteur audio
                    audio_output = aud_writeinit(sample_rate, sample_size, channels); 
                    if(audio_output < 0)
                        die("Erreur lors de l'ouverture du device audio");
                    
                    printf("Lecture de %s en cours\n", argv[2]);
                    
                    // demande de la suite du morceau
                    packet_send.type = NEXT_BLOCK;
                    break;

                case BLOCK:
                    // lecture du paquet reçu
                    if(write(audio_output, packet_received.data, BUFF_SIZE) <0 )
                        die("erreur lors de l'écritue dans la sortie audio");

                    // demande de la suite du morceau
                    packet_send.type = NEXT_BLOCK;
                    break;
                
                default: //EOF ou ERROR
                    close(audio_output);
                    end = 1;
                    printf("%s\n", packet_received.data);
                    //envoi du dernier paquet
                    packet_send.type = CLOSE_CNX;
                    break;
            }
            if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr*) &serv_addr, flen) < 0)
                die("Erreur lors de l'envoie");
        } // Fin si timeout ok
    } // fin while

    close(socketfd);
    return 0;
}