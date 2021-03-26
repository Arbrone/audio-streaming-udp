#ifndef PACKET_H_
#define PACKET_H_

// Type paquet
#define EMPTY       0
#define HEADER      1
#define FILENAME    2
#define BLOCK       3
#define NEXT_BLOCK  4
#define RESEND      5
#define END         6
#define CLOSE_CNX   7
#define PORT        8
#define ERROR       -1

#define BUFF_SIZE   1024

typedef struct packet
{
    int type;
    char data[BUFF_SIZE];
} packet;

void init_packet(packet *packet, int type, char* data);

void clear_packet(packet *packet);

#endif