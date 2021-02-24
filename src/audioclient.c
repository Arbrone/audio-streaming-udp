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
    
    if(argc != 3) { 
        die("Respecter le format : ./client server_host_name file_name");
    }

    /* #### VARIABLES ####*/
    // Network
    struct sockaddr_in serv_addr;
    packet packet_send;
    packet packet_received;
    socklen_t len;
    socklen_t flen = sizeof(struct sockaddr_in);
    int socketfd;
    

    //Audio
    int sample_rate, sample_size, channels;
    int audio_output;
    int end = 0;
    int get = 0;
    int send = 0;
    int longueur = 0;

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

    if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr*) &serv_addr, flen) < 0)
        die("Erreur lors de l'envoie");
    send++;

    
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
            send++;
        }

        if(FD_ISSET(socketfd, &read_set)){
            // remise à 0 des paquets à envoyer
            clear_packet(&packet_send);
            clear_packet(&packet_received);
            
            if(get != 90){
                printf("get = %d\n", get);
                // reception d'un nouveau paquet
               if(recvfrom(socketfd, &packet_received, sizeof(packet), 0, (struct sockaddr*) &serv_addr, &flen) < 0)
                    die("Erreur lors de la reception");
            }else{
                printf(RED"paquet non reçu\n"RESET);
            }
            get++;

            switch (packet_received.type)
            {
                case HEADER:
                    // récupération des metadata
                    parseMetadata(packet_received.data, &sample_rate, &sample_size, &channels);

                    // et initialisation du lecteur audio
                    //TODO comment for WSL test
                    /*audio_output = aud_writeinit(sample_rate, sample_size, channels); 
                    if(audio_output < 0)
                        die("Erreur lors de l'ouverture du device audio");*/
                    
                    printf("Lecture de %s en cours\n", argv[2]);
                    
                    // demande de la suite du morceau
                    packet_send.type = NEXT_BLOCK;
                    break;
                
                case BLOCK:
                    // lecture du paquet reçu
                    //TODO comment for WSL test
                    /*if(write(audio_output, packet_received.data, BUFF_SIZE) <0 )
                        die("erreur lors de l'écritue dans la sortie audio");*/
                    
                    longueur += strlen(packet_received.data);
                    // demande de la suite du morceau
                    packet_send.type = NEXT_BLOCK;
                    break;
                
                case EMPTY:
                     init_packet(&packet_send,RESEND,"timeout");
                        // on redemande le meme packet
                        if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr*) &serv_addr, flen) < 0)
                            die("Erreur lors de l'envoie");
                        break;
                
                default: //EOF ou ERROR
                    //TODO comment for WSL test
                    //close(audio_output);
                    end = 1;
                    
                    //envoi du dernier paquet
                    packet_send.type = CLOSE_CNX;
                    break;
            }

            if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr*) &serv_addr, flen) < 0)
                die("Erreur lors de l'envoie");
            send++;
        } // Fin si timeout ok
    } // fin while


    close(socketfd);

    // verifications envoi correct
    printf("Paquet envoyés : %d\n",send);
    printf("Paquet reçus : %d\n",get);
    printf("Taille : %d\n",longueur);

    return 0;
}