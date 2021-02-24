#include <string.h>
#include "../include/packet.h"

void init_packet(packet *packet, int type, char* data){
    packet->type = type;
    strcpy(packet->data, data);
}

void clear_packet(packet *packet){
    packet->type = EMPTY;
    memset(packet->data, 0, BUFF_SIZE);
}