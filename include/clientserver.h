#ifndef CLIENTSERVER_H_
#define CLIENTSERVER_H_

// Include
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "packet.h"
#include "lib/sysprog-audio-1.5/audio.h"

struct in_addr* resolve_host(char  *hostname);


/**
 * Kill the program
 * @param message   message to display
*/
void die(char *message);


/** Initialise le socket côté serveur
 * @param addr      un pointeur vers l'adresse du serveur
 * @param port      le port du socket
 * @return          le fd du socket, -1 sinon.
 */
int init_server_socket (struct sockaddr_in *serv_addr, uint16_t server_port);

/** Initialise une socket pour le client
 * @param addr      un pointeur vers l'adresse du serveur
 * @param port      le port sur lequel joindre le serveur
 * @param hostname  l'adresse du serveur sous la forme X.X.X.X
 * @return          le fd du socket, -1 sinon.
 */
int init_client_socket (struct sockaddr_in *serv_addr,  uint16_t server_port, char *hostname);

/** Initialise une socket pour le processus enfant
 * @param addr      un pointeur vers l'adresse du serveur parent
 * @return          le fd du socket, -1 sinon.
 */
int init_child_socket(struct sockaddr_in *serv_addr);

/** Permet d'extraire les metadata du fichier wav lu
 * @param sample_rate   un pointeur pour stocker le rate
 * @param sample_size   un pointeur pour stocker la taille
 * @param channels      un pointeur pour stocker channels (mono / stereo)
 */
void parseMetadata(char *buffer, int *sample_rate, int *sample_size, int *channels);

/** Creer un nouveau processus enfant
 * @param nb_client   le nombre de clients actuellement connectés
 * @param max_client  le nombre maximum de connexions autorisées
 * @return            le pid de l'enfant
 */
int new_client_process(int *nb_client, int max_client);


/** Traite le paquet reçu et prépapre celui à envoyer
 * @param packet_received   la paquet à traiter
 * @param end               un pointeur mis à 1 si CLOSE_CNX      
 * @return                  le packet de réponse
 */
packet get_packet_to_send(packet packet_received, int *audio_fd, int *sample_rate, int *sample_size, int *channels, int *end);

#endif