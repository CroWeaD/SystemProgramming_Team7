#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>


#define CITY              0         // 도시
#define SPECIAL           1         // 특수
#define GOLDED_KEY        2         // 황금열쇠

#define START             0
#define ISLAND_SQR        7         // 무인도 칸
#define SOCIAL_FUND_SQR  14         // 사회기금 칸
#define SPACE_TRAVEL_SQR 21         // 우주여행 칸
#define SPACESHIP        23         // 우주선 칸
#define DONATION_SQR     26         // 기부 칸
#define TOTAL_SQR        28         // 전체 칸

#define TUSD              10
#define SALARY       20*TUSD

#define HOTEL             0
#define BUILDING          1
#define VILLA             2
#define LAND              3

#define B_HOTEL      0b1000
#define B_BUILDING   0b0100
#define B_VILLA      0b0010
#define B_LAND       0b0001

// 칸
typedef struct Square {
    int type;       // 0: 도시, 1: 특수도시, 2: 황금열쇠 3: 기타
    int buildings;  // hotel: 1000, building: 0100, villa: 0010, land: 0001
    int owner;
    const int price[4]; // hotel, building, villa, land
    const int toll[4];  // hotel, building, villa, land
} Square;

// 플레이어
typedef struct Player {
    int id;                     // player ID
    int cash;                   // 소지금
    int pos;                    // position
    int island_remaining;       // 무인도 남은 턴 수
    bool has_loan;              // 대출 여부
    bool has_voucher;           // 우대권 소유 여부
    bool has_escape;            // 무인도 탈출권 소유 여부
    bool paid_for_sship;        // 우주왕복선 비용 지불 여부
} Player;
// game
void start_game(int pnum, int clnt[4], char *n1, char*n2, char*n3, char*n4) ;
void set_up();
void start_turn();
void roll_dice(int doubleNum);
void arrived_square(int curr_pos);
void arrived_city(int type, int curr_pos);
void arrived_etc();
int move_player(int move);
int real(int p);
int cal_toll(int curr_pos);
int cal_price(int curr_pos);
void bankruptcy();
void trading(int curr_pos, int total_price, int seller, int buyer);
void checkBitsSet(int bitmask1, int bitmask2, int currPlayer);
int construction(int receipt, int currPlayer, int curr_pos, int building_type, int building_bit);
void arrived_golden_key(int curr_pos);
int* tax(int curr_pos, int* arr);
void shuffle();