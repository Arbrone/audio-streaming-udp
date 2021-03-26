/**
 * Projet SYS, rendu du 26 mars
 * Binome : Thomas VERRIER, Alex GILLES
 * Groupe : 1.1 
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "../include/clientserver.h"
#include "../include/packet.h"
#include "../include/clientserver.h"
#include "../include/lib/sysprog-audio-1.5/audio.h"

#define RED "\x1B[31m"
#define BLUE "\x1b[34m"
#define GRE "\x1b[32m"
#define RESET "\x1B[0m"
#define MAX_CLIENT 2

int NB_CLIENT;

void sig_chld(int sig)
{
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
        NB_CLIENT--;
        printf("Child %d terminated with status %d\n", pid, stat);
    }
}

int main()
{
    printf("# " GRE "Lancement du serveur\n" RESET "#\n");

    /* ##### VARIABLES ##### */
    // Network
    struct sockaddr_in serv_addr, client_addr;
    packet packet_send, packet_received;
    socklen_t len;
    socklen_t flen = sizeof(struct sockaddr_in);
    int socketfd, child_socket;

    // Audio
    int sample_rate, sample_size, channels;
    int audio_file;

    // MULTI CLIENT
    NB_CLIENT = 0;
    int pid = getpid();

    // Handler sig_child
    struct sigaction handler;
    handler.sa_handler = sig_chld;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    sigaction(SIGCHLD, &handler, NULL);

    /* #### NETWORK INITIALISATION #### */
    socketfd = init_server_socket(&serv_addr, 1234);
    if (socketfd < 0)
        die("erreur socket");

    /* #### DEBUT COMMUNICATION #### */
    // le serveur ne s'arrête que sur erreur
    while (1)
    {
        // le serveur se met en attente d'un nouveau client
        len = recvfrom(socketfd, &packet_received, sizeof(packet), 0, (struct sockaddr *)&client_addr, &flen);
        if (len < 0)
        {
            die("Erreur lors de la réception");
        }

        if (packet_received.type == FILENAME)
        {
            printf(BLUE"Connexion : @ %s Port : %d\n" RESET, inet_ntoa(client_addr.sin_addr), htons(serv_addr.sin_port));

            // nombre de connexions max atteint
            // lancemement d'un nouveau processus si le serveur n'est pas full
            if ((pid = new_client_process(&NB_CLIENT, MAX_CLIENT)) < 0)
            {
                init_packet(&packet_send, ERROR, "nb utilisateurs max atteint");
                if (sendto(socketfd, &packet_send, sizeof(packet), 0, (struct sockaddr *)&client_addr, flen) < 0)
                    die(RED "Erreur lors de l'envoie\n" RESET);
            }

            // si nombre de clients ilimité
            /*pid = fork();
            if (pid < 0)
            {
                die("Erreur création enfant");
            }*/

            NB_CLIENT++;

            // PROCESSUS ENFANT
            if (pid == 0)
            {
                int child_end = 0;
                child_socket = init_child_socket(&serv_addr);

                printf(BLUE"Affectation : @ %s Port : %d\n"RESET, inet_ntoa(client_addr.sin_addr), htons(serv_addr.sin_port));
                printf("processus enfant : %d\n", getpid());

                /* #### DEBUT ECHANGE FICHIER #### */
                while (!child_end)
                {
                    //traitement du paquet reçu
                    packet_send = get_packet_to_send(packet_received, &audio_file, &sample_rate, &sample_size, &channels, &child_end);

                    // verification pour bien traiter le cas CLOSE_CNX et ne pas rentrer dans un recvfrom d'un paquet qui n'arrivera pas
                    // child_end sera mis à vrai par get_packet_to_send() en cas de CLOSE_CNX
                    if (!child_end)
                    {
                        if (sendto(child_socket, &packet_send, sizeof(packet), 0, (struct sockaddr *)&client_addr, flen) < 0)
                            die(BLUE "Erreur lors de l'envoie\n" RESET);

                        len = recvfrom(child_socket, &packet_received, sizeof(packet), 0, (struct sockaddr *)&client_addr, &flen);
                        if (len < 0)
                            die("Erreur lors de la réception");
                    }
                }

                close(child_socket);
                return 0;
            } //fin enfant
        }
        clear_packet(&packet_received);
        //printf("nb client fin while %d\n", NB_CLIENT);
    } //fin while
    close(socketfd);

    return 0;
} // fin main

//? clear && echo -e '   PPID     PID    PGID     SID TTY        TPGID STAT   UID   TIME COMMAND' && ps -axjf | grep "./server"
