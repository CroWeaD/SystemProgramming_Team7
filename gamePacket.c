#include "gamePacket.h"

Game_Send send_packet;
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


void packing2Send(int clnt, int type, int player, int* data) {
    memset(&send_packet, 0, sizeof(send_packet));
    send_packet.type = type;
    send_packet.player = player;
    memmove(send_packet.data, data, sizeof(int)*10);    
    write(clnt_sock[clnt], (void*)&send_packet, sizeof(send_packet));
}

// send name
void sendName(int playerNum, int currPlayer, char* name0, char* name1, char* name2, char* name3) {
    Name_Send name_send;
    memset(&name_send, 0, sizeof(name_send));
    name_send.player = currPlayer;
    name_send.playernum = playerNum;
    memcpy(name_send.name0, name0, sizeof(char) * 10);
    memcpy(name_send.name1, name1, sizeof(char) * 10);
    memcpy(name_send.name2, name2, sizeof(char) * 10);
    memcpy(name_send.name3, name3, sizeof(char) * 10);
    write(clnt_sock[currPlayer], (void*)&name_send, sizeof(name_send));

    printf("sent name to %d player.\n",currPlayer);    
}

//type 0
void sendTravelResult(int playerNum, int currPlayer, int moveto, int playerCash) {
    int data[10] = {0,};
    data[0] = currPlayer; data[1] = moveto; data[2] = playerCash;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 0, currPlayer, data);
}
//type 1
void sendDiceWait(int playerNum, int currPlayer) {
    int data[10] = {0,};
    int clnt = currPlayer;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 1, currPlayer, data);
}
//type 2
void sendDiceResult(int playerNum, int currPlayer, int die1, int die2, int moveto, int doubleNum, int playerCash) {
    int data[10] = {0,};
    data[0] = die1; data[1] = die2; data[2] = moveto; data[3] = doubleNum; data[4] = playerCash;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 2, currPlayer, data);
}
//type 3
void sendDouble3Time(int playerNum, int currPlayer, int moveto) {
    int data[10] = {0,};
    data[0] = currPlayer; data[1] = moveto;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 3, currPlayer, data);
}
//type 4
void sendChooseLand2Buy(int playerNum, int currPlayer, int curr_pos, int buildings) {
    int data[10] = {0,};
    int clnt = currPlayer;
    data[0] = curr_pos; data[1] = buildings;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 4, currPlayer, data);
}
//type 5
void sendTakeOverWait(int playerNum, int currPlayer, int curr_pos, int cost, int host) {
    int data[10] = {0,};
    int clnt = currPlayer;
    data[0] = curr_pos; data[1] = cost; data[2] = host;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 5, currPlayer, data);
}
//type 6
void sendTakeOverResult(int playerNum, int currPlayer, int curr_pos, int host, int cost, int visitantCash, int hostCash) {
    int data[10] = {0,};
     data[0] = curr_pos; data[1] = currPlayer; data[2] = host; data[3] = cost; data[4] = visitantCash; data[5] = hostCash;
    for(int i = 0; i<playerNum; i++)
        packing2Send(i, 6, currPlayer, data);
}
//type 7
void sendPayResult(int playerNum, int currPlayer, int host, int cost, int visitantCash, int hostCash) {
    int data[10] = {0,};
    data[0] = currPlayer; data[1] = host; data[2] = cost; data[3] = visitantCash; data[4] = hostCash;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 7, currPlayer, data);
}
//type 8
void sendWillLoan(int playerNum, int currPlayer, int curr_pos, int host, int cost) {
    int data[10] = {10,};
    int clnt = currPlayer;
    data[0] = curr_pos; data[1] = host; data[2] = cost;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 8, currPlayer, data);
}
//type 9
void sendLoanMsg(int playerNum, int currPlayer) {
    int data[10] = {0,};
    data[0] = currPlayer;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 9, currPlayer, data);
}
//type 10
void sendChooseLand2Sell(int playerNum, int currPlayer, int curr_pos, int cost, int ownLand, 
                        int building1, int building2, int building3, int building4) {
    int data[10] = {0,};
    int clnt = currPlayer;
    data[0] = curr_pos; data[1] = cost; data[2] = ownLand; data[3] = building1; data[4] = building2; data[5] = building3; data[6] = building4;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 10, currPlayer, data);
}
//type 11
void sendAskTravel(int playerNum, int currPlayer, int host, int cost) {
    int data[10] = {0,};
    int clnt = currPlayer;
    data[0] = host; data[1] = currPlayer; data[2] = cost;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 11, currPlayer, data);
}
//type 12
void sendWhere2Travel(int playerNum, int currPlayer) {
    int data[10] = {0,};
    int clnt = currPlayer;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 12, currPlayer, data);
}
//type 13
void sendIslandEscape(int playerNum, int currPlayer, bool escape, int remaining) {
    int data[10] = {0,};
    int has_escape = 0;
    if(escape)  has_escape = 1;
    data[0] = has_escape; data[1] = remaining;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 13, currPlayer, data);
}
//type 14
void sendIslandDiceWait(int playerNum, int currPlayer, int islandRemain) {
    int data[10] = {0,};
    data[0] = islandRemain;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 14, currPlayer, data);
}
//type 15
void sendIslandDiceResult(int playerNum, int currPlayer, int die1, int die2, int moveto) {
    int data[10] = {0,};
    data[0] = die1; data[1] = die2; data[2] = moveto;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 15, currPlayer, data);
}
//type 16
void sendReceivingFunds(int playerNum, int currPlayer, int fund, int playerCash) {
    int data[10] = {0,};
    data[0]= currPlayer; data[1] = fund; data[2] = playerCash;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 16, currPlayer, data);
}
//type 17
void sendDonation(int playerNum, int currPlayer, int donation, int fund, int playerCash) {
    int data[10] = {0,};
    data[0] = currPlayer; data[1] = donation; data[2] = fund; data[3] = playerCash;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 17, currPlayer, data);
}
//type 18
void sendBankruptcy(int playerNum, int currPlayer, int winner) {
    int data[10] = {0,};
    data[0] = currPlayer; data[1] = winner;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 18, currPlayer, data);
}
//type 19
void sendAskVoucher(int playerNum, int currPlayer) {
    int data[10] = {0,};
    int clnt = currPlayer;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 19, currPlayer, data);
}
//type 20
void sendUseVoucher(int playerNum, int currPlayer) {
    int data[10] = {0,};
    data[0] = currPlayer;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 20, currPlayer, data);
}
//type 21
void sendLandPuchaseResult(int playerNum, int currPlayer, int curr_pos, int price, int building_info, int playerCash, int toll) {
    int data[10] = {0,};
    data[0] = curr_pos; data[1] = currPlayer; data[2] = price; data[3] = building_info; data[4] = playerCash; data[5] = toll;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 21, currPlayer, data);
}
//type 22
void sendLandSellResult(int playerNum, int currPlayer, int sellLand, int playerCash) {
    int data[10] = {0,};
    data[0] = currPlayer; data[1] = sellLand; data[2] = playerCash;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 22, currPlayer, data);
}
//type 23
void sendGoldenKey(int playerNum, int currPlayer, int key_num, int data1, int data2, int data3, int data4, int data5) {
    int data[10] = {0,};
    data[0] = key_num; data[1] = data1; data[2] = data2; data[3] = data3; data[4] = data4; data[5] = data5;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 23, currPlayer, data);
}