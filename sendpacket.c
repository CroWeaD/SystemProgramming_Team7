#include "gamePacket.h"

Game_Send send_packet;

void packing2Send(int clnt, int type, int player, int* data) {
    memset(&send_packet, 0, sizeof(send_packet));
    send_packet.type = type;
    send_packet.player = player;
    memmove(send_packet.data, data, sizeof(int)*10);    
    write(clnt_sock[clnt], (void*)&send_packet, sizeof(send_packet));
}

//type 0
void sendSync(int playerNum, int currPlayer, int p1Pos, int p2Pos, int p3Pos, int p4Pos, 
            int p1Cash, int p2Cash, int p3Cash, int p4Cash) {
    int data[10] = {0,};
    data[0] = p1Pos; data[1] = p2Pos; data[2] = p3Pos; data[3] = p4Pos;
    data[4] = p1Cash; data[5] = p2Cash; data[6] = p3Cash; data[7] = p4Cash;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 0, currPlayer, data);
}
//type 1
void sendDiceWait(int currPlayer) {
    int data[10] = {0,};
    int clnt = currPlayer;
    packing2Send(clnt, 1, currPlayer, data);
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
void sendChooseLand2Buy(int currPlayer, int curr_pos, int buildings) {
    int data[10] = {0,};
    int clnt = currPlayer;
    data[0] = curr_pos; data[1] = buildings;
    packing2Send(clnt, 4, currPlayer, data);
}
//type 5
void sendTakeOverWait(int currPlayer, int curr_pos, int cost, int host) {
    int data[10] = {0,};
    int clnt = currPlayer;
    data[0] = curr_pos; data[1] = cost; data[2] = host;
    packing2Send(clnt, 5, currPlayer, data);
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
void sendWillLoan(int currPlayer, int curr_pos, int host, int cost) {
    int data[10] = {10,};
    int clnt = currPlayer;
    data[0] = curr_pos; data[1] = host; data[2] = cost;
    packing2Send(clnt, 8, currPlayer, data);
}
//type 9
void sendLoanMsg(int playerNum, int currPlayer) {
    int data[10] = {0,};
    data[0] = currPlayer;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 9, currPlayer, data);
}
//type 10
void sendChooseLand2Sell(int currPlayer, int curr_pos, int cost, int ownLand, 
                        int building1, int building2, int building3, int building4) {
    int data[10] = {0,};
    int clnt = currPlayer;
    data[0] = curr_pos; data[1] = cost; data[2] = ownLand; data[3] = building1; data[4] = building2; data[5] = building3; data[6] = building4;
    packing2Send(clnt, 10, currPlayer, data);
}
//type 11
void sendAskTravel(int currPlayer, int host, int cost) {
    int data[10] = {0,};
    int clnt = currPlayer;
    data[0] = host; data[1] = currPlayer; data[2] = cost;
    packing2Send(clnt, 11, currPlayer, data);
}
//type 12
void sendWhere2Travel(int currPlayer) {
    int data[10] = {0,};
    int clnt = currPlayer;
    packing2Send(clnt, 12, currPlayer, data);
}
//type 13
void sendIslandEscape(int currPlayer, bool escape, int remaining) {
    int data[10] = {0,};
    int has_escape = 0;
    if(escape)  has_escape = 1;
    data[0] = has_escape; data[1] = remaining;
    sendpacking(13, currPlayer, data);
}
//type 14
void sendIslandDiceWait(int currPlayer) {
    int data[10] = {0,};
    int clnt = currPlayer;
    packing2Send(clnt, 14, currPlayer, data);
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
void sendAskVoucher(int currPlayer) {
    int data[10] = {0,};
    int clnt = currPlayer;
    packing2Send(clnt, 19, currPlayer, data);
}
//type 20
void sendUseVoucher(int playerNum, int currPlayer) {
    int data[10] = {0,};
    data[0] = currPlayer;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 20, currPlayer, data);
}
//type 21
void sendLandPuchaseResult(int playerNum, int currPlayer, int curr_pos, int price, int building_info, int playerCash) {
    int data[10] = {0,};
    data[0] = curr_pos; data[1] = currPlayer; data[2] = price; data[3] = building_info; data[4] = playerCash;
    for(int i=0; i<playerNum; i++)
        packing2Send(i, 21, currPlayer, data);
}