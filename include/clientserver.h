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
#include "../include/packet.h"

// Constants
#define LOCALHOST   "127.0.0.1"

// Methods

struct in_addr* resolve_host(char  *hostname);


/**
 * Kill the program
 * @param message   message to display in the console
*/
void die(char *message);


/** Initialise le socket
 * @param addr      un pointeur vers l'adreesse du serveur
 * @param port      le port du socket
 * @return          le fd du socket, -1 sinon.
 */
int init_server_socket (struct sockaddr_in *serv_addr, uint16_t server_port);

int init_client_socket (struct sockaddr_in *serv_addr,  uint16_t server_port, char *hostname);

void parseMetadata(char *buffer, int *sample_rate, int *sample_size, int *channels);

#endif