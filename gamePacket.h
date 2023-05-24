#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

// 닉네임 전송 패킷
typedef struct Name_Send {
    int player;
    int playernum;
    char name0[10];
    char name1[10];
    char name2[10];
    char name3[10];
} Name_Send;

// 발신 패킷
typedef struct Game_Send {
    int type;
    int player;
    int data[10];
} Game_Send;

// 수신 패킷
typedef struct Game_Recv {
    int type;
    int player;
    int data[2];
} Game_Recv;

extern int serv_sock;
extern int clnt_sock[4];

// RECV PACKET
int recvPack(int playerNum, int clnt);

void packing2Send(int clnt, int type, int player, int* data);

// SEND PACKET
void sendName(int playerNum, int currPlayer, char* name1, char* name2, char* name3, char* name4);
void sendSync(int playerNum, int currPlayer, int p1Pos, int p2Pos, int p3Pos, int p4Pos, int p1Cash, int p2Cash, int p3Cash, int p4Cash);
void sendDiceWait(int playerNum, int currPlayer);
void sendDiceResult(int playerNum, int currPlayer, int die1, int die2, int moveto, int doubleNum, int playerCash);
void sendDouble3Time(int playerNum, int currPlayer, int moveto);
void sendChooseLand2Buy(int playerNum, int currPlayer, int curr_pos, int buildings);
void sendTakeOverWait(int playerNum, int currPlayer, int curr_pos, int cost, int host);
void sendTakeOverResult(int playerNum, int currPlayer, int curr_pos, int host, int cost, int visitantCash, int hostCash);
void sendPayResult(int playerNum, int currPlayer, int host, int cost, int visitantCash, int hostCash);
void sendWillLoan(int playerNum, int currPlayer, int curr_pos, int host, int cost);
void sendLoanMsg(int playerNum, int currPlayer);
void sendChooseLand2Sell(int playerNum, int currPlayer, int curr_pos, int cost, int ownLand, int building1, int building2, int building3, int building4);
void sendAskTravel(int playerNum, int currPlayer, int host, int cost); 
void sendWhere2Travel(int playerNum, int currPlayer);
void sendIslandEscape(int playerNum, int currPlayer, bool escape, int remaining);
void sendIslandDiceResult(int playerNum, int currPlayer, int die1, int die2, int moveto);
void sendReceivingFunds(int playerNum, int currPlayer, int fund, int playerCash);
void sendDonation(int playerNum, int currPlayer, int donation, int fund, int playerCash);
void sendBankruptcy(int playerNum, int currPlayer, int winner);
void sendAskVoucher(int playerNum, int currPlayer);
void sendUseVoucher(int playerNum, int currPlayer);
void sendLandPuchaseResult(int playerNum, int currPlayer, int curr_pos, int price, int building_info, int playerCash, int toll);

