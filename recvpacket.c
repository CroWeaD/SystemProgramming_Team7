#include "gamePacket.h"

Game_Recv recv_packet[4];

int recvPack(int playerNum, int clnt) {
    if(playerNum!=1) {
        for(int i=0; playerNum; i++)
            memset(&recv_packet[i], 0, sizeof(recv_packet[i]));
        for(int i=0; i<playerNum; i++) {
            read(clnt_sock[i], (void*)&recv_packet[i], sizeof(recv_packet[i]));
            printf("[recv] player: %d, type: %d\n", i, recv_packet[i].type);
        }
        return 1;
    }

    memset(&recv_packet[clnt], 0, sizeof(recv_packet[clnt]));
    read(clnt_sock[clnt], (void*)&recv_packet[clnt], sizeof(recv_packet[clnt]));

    int type = recv_packet[clnt].type;
    int data = recv_packet[clnt].data[0];
    printf("[recv] player: %d, type: %d\n", clnt, type);
    switch(type) {
        case 0: return 1;
        case 1: case 2: case 3: case 4: return data;
        default:
            printf("invalid type");
            close(clnt_sock[clnt]);
    }    
}
