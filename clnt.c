#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "game.h"

#define SERVER_IP   "127.0.0.1"

static int sock;

static RECV_PACKET packet_recv;
static SEND_PACKET packet_send;
static INIT_PACKET init_packet;

void setupServer(int serv_port){    
    struct sockaddr_in serv_addr;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(serv_port);

    if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != 0){
        perror("connect() error!");
        exit(1);
    }

    printf("connected to server.\n");
    read(sock,(void*)&init_packet,sizeof(INIT_PACKET));
}

void process_packet(RECV_PACKET* packet){
    gameAction(packet->player, packet->data, packet->type,&packet_send);
    write(sock,(void*)&packet_send,sizeof(SEND_PACKET));
}

void get_response(RECV_PACKET* packet){
    read(sock,(void*)packet,sizeof(RECV_PACKET));
}

void set_dummydata(){
    for(int i = 0;i<4;i++){
        strncpy(players[i].name,init_packet.names[i],10);
        players[i].position = 0;
        players[i].budget = 6600;
    }
}

int main(int argc, char* argv[]){
    setupServer(8080);
    set_dummydata();
    startGame(init_packet.player, init_packet.player_num);
    while(true){
        get_response(&packet_recv);
        process_packet(&packet_recv);
    }
    endGame();
}