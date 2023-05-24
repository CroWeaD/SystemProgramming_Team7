#include "gamePacket.h"

Game_Recv recv_packet[4];

int recvPack(int playerNum, int clnt) {
    int result = 0;
    for(int i=0; i<playerNum; i++) {
        memset(&recv_packet[i], 0, sizeof(recv_packet[i]));
        read(clnt_sock[i], (void*)&recv_packet[i], sizeof(recv_packet[i]));
        if(i != clnt)  continue;

        int type = recv_packet[i].type;
        int data = recv_packet[i].data[0];
        printf("[recv] player: %d, type: %d, data: %d\n", i, type, data);
        switch(type) {
            case 0: 
                result = 1; break;
            case 1: case 2: case 3: case 4:  
                result = data;  break;
            default:
                printf("invalid type");
                close(clnt_sock[i]);
        }
    } 
    return result;
}
