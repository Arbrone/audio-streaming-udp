#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/lib/sysprog-audio-1.5/audio.h"


int main(int argc,char *argv[]){

    int nb;
    int taille = 0;

    char filename[128];
    if(argc != 2) { 
        printf("Nombre d'arguments incorrect\n");
        return -1;
    }

    strcpy(filename, argv[1]);

    int sample_rate, sample_size, channels;

    int fd = aud_readinit(filename, &sample_rate, &sample_size, &channels);
    if(fd < 0) {
        perror("erreur readinit"); 
        return -1;
    }      

    int fd2 = aud_writeinit(sample_rate, sample_size, channels);
    if(fd2 < 0) { 
        perror("erreur writeinit");
        return -1;
    }

    char buffer[1024];
    while(read(fd, buffer, 1024) != 0){
        nb++;
        taille += strlen(buffer);
        write(fd2, buffer, 1024);
    }

    close(fd);
    close(fd2);
    
    printf("%s\n", "Fin de lecture");
    printf("nombre de paquet necessaire : %d\n", nb);
    printf("Taille : %d\n", taille);

    return 0;
}

// Question 1
// sample_rate = 88200  -> 2x plus rapide 
// sample_rate = 22050  -> 2x plus lent

// Question 2 | 2 = stereo (source de base) 1= mono
// channels = 1;
// on perd un partie du signal donc moins de nuances

// Question 3
// sample_size = 8;
// A NE PAS FAIRE LE SON ENCULE LES OREILLES !!!!