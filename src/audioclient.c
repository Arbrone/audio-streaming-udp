#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../include/clientserver.h"
#include "../include/packet.h"
#include "../include/lib/sysprog-audio-1.5/audio.h"

#define RED   "\x1B[31m"
#define GRE   "\x1b[32m"
#define RESET "\x1B[0m"


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

        // remise à 0 des paquets à envoyer
        clear_packet(&packet_send);
        clear_packet(&packet_received);

        // reception d'un nouveau paquet
        len  = recvfrom(socketfd, &packet_received, sizeof(packet), 0, (struct sockaddr*) &serv_addr, &flen);
        if(len < 0)
            die("Erreur lors de la reception");
        get++;

        switch (packet_received.type)
        {
            case HEADER:
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
                
                longueur += strlen(packet_received.data);
                // demande de la suite du morceau
                packet_send.type = NEXT_BLOCK;
                break;
            
            case RESEND:
            break
            
            default: //EOF ou ERROR
                close(audio_output);
                end = 1;
                
                //envoi du dernier paquet
                packet_send.type = CLOSE_CNX;
                break;
        }

        if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr*) &serv_addr, flen) < 0)
            die("Erreur lors de l'envoie");
        send++;
    }


    close(socketfd);

    // verifications envoi correct
    printf("Paquet envoyés : %d\n",send);
    printf("Paquet reçus : %d\n",get);
    printf("Taille : %d\n",longueur);

    return 0;
}
