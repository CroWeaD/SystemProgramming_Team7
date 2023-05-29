#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "game_clnt.h"
#include <ncursesw/curses.h>

#define SERVER_IP   "127.0.0.1"


static RECV_PACKET packet_recv;
static SEND_PACKET packet_send;
static INIT_PACKET init_packet;

void process_packet(RECV_PACKET* packet, int sock){
    gameAction(packet->player, packet->data, packet->type,&packet_send);
    write(sock,(void*)&packet_send,sizeof(SEND_PACKET));
}

void get_response(RECV_PACKET* packet,int sock){
    read(sock,(void*)packet,sizeof(RECV_PACKET));
}

void set_data(){
    for(int i = 0;i<4;i++){
        strncpy(players[i].name,init_packet.names[i],20);
        players[i].position = 0;
        players[i].budget = 6600;
    }
}

void mainLoop(int serv_socket){
    int sock = serv_socket;
    int readlen;
    int t = 0;
    int t2 = 0;
    char log[256];
    readlen = read(sock,(void*)&init_packet,sizeof(INIT_PACKET));
    set_data();
    startGame(init_packet.player, init_packet.player_num);
    while(true){
        get_response(&packet_recv,sock);
        process_packet(&packet_recv,sock);
        if(packet_recv.type == 18)
            break;
    }
    endGame();
}