#pragma once
#include <ncursesw/curses.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <wchar.h>
#include <string.h>


typedef struct{
    int budget;
    char name[20];
    int position;
}Player;

typedef struct RECV_PACKET{
    int type;
    int player;
    int data[10];
} RECV_PACKET;

typedef struct SEND_PACKET{
    int type;
    int data[2];
} SEND_PACKET;

typedef struct INIT_PACKET{
    int player;
    int player_num;
    char names[4][20];
} INIT_PACKET;

#define COLOR_PAIR_NUM 16
#define COLOR_PAIR_DEAFULT 1
#define COLOR_PAIR_BLOCK_YELLOW 2
#define COLOR_PAIR_BLOCK_YELLOW_LIGHT 3
#define COLOR_PAIR_BLOCK_GRAY 4
#define COLOR_PAIR_BLOCK_GRAY_LIGHT 5
#define COLOR_PAIR_BLUE 6
#define COLOR_PAIR_GREEN 7
#define COLOR_PAIR_P1 8
#define COLOR_PAIR_P1_ALT 9
#define COLOR_PAIR_P2 10
#define COLOR_PAIR_P2_ALT 11
#define COLOR_PAIR_P3 12
#define COLOR_PAIR_P3_ALT 13
#define COLOR_PAIR_P4 14
#define COLOR_PAIR_P4_ALT 15
#define COLOR_PAIR_WHITE 254
#define COLOR_PAIR_GRAY_DARK 255
#define COLOR_PAIR_HIGHLIGHT 249
#define COLOR_PAIR_FADE 248

#define COLOR_PAIR_P1_DARK 250
#define COLOR_PAIR_P2_DARK 251
#define COLOR_PAIR_P3_DARK 252
#define COLOR_PAIR_P4_DARK 253

#define BLOCK_YELLOW_BACK 228
#define BLOCK_YELLOW_LIGHT_BACK 229
#define BLOCK_GRAY_BACK 253
#define BLOCK_GRAY_LIGHT_BACK 255
#define BLOCK_BLUE_BACK 75
#define BLOCK_GREEN_BACK 10
#define BLOCK_DARK_GRAY 242

#define P1_MARKER 88            // red
#define P2_MARKER 18            // blue
#define P3_MARKER 94            // yellow
#define P4_MARKER 22            // green

#define BLOCK_P1_BACK 217
#define BLOCK_P1_ALT_BACK 216
#define BLOCK_P2_BACK 44
#define BLOCK_P2_ALT_BACK 43
#define BLOCK_P3_BACK 222
#define BLOCK_P3_ALT_BACK 221
#define BLOCK_P4_BACK 107
#define BLOCK_P4_ALT_BACK 108

#define T_N 0
#define T_KEY 1
#define T_START 2
#define T_ISLAND 3
#define T_WELFARE 4
#define T_SPACE 5
#define T_SPECIAL 6

extern int maxindex;
extern Player players[4];
extern int player_count;
extern int cost[28];
extern int owner[28]; 
extern int buildings[28];

extern int buildingPrice[28][4];
extern int buildingToll[28][4];

extern WINDOW* mainWindow;

extern int w;
extern int h;

extern wchar_t* names[];

void setOkayButton(wchar_t* text, int color);
void setTwoButtons(wchar_t* text1, wchar_t* text2, int color1, int color2);
void clearContext();

void drawPlayerInfo(int player);
int getColorPair(int player, int cPair);
int getPlayerColorPair(int player, bool alt);
void drawBuilding(int building, int x, int y, int player, int color);
void setHeader(wchar_t* text, bool show, int color);
void drawDiceBoard(int color);
void drawDice(int die1, int die2,int color);
void setLandSaleContext(wchar_t* name, bool enable_land, bool enable_b1, bool enable_b2, bool enable_b3, 
            int price[4], int toll[4], bool toggle_b1, bool toggle_b2, bool toggle_b3, 
            int selection, int normal_color, int selected_color, int fade_color);
void setTollContext(wchar_t* owner, wchar_t* name, int toll);

void movePlayer(int movingID, int movingPosition);
bool gameAction(int player, int* data, int type, SEND_PACKET* packet);
void highlightTile(int index, bool highlight);
void drawTile(int index);
void drawPrice(int price, int x, int y, int color);
void drawMarker(bool p1, bool p2, bool p3, bool p4, int x, int y, int color);
void drawbox(int x, int y, int color);
void drawLabel(wchar_t* text, int x, int y, int color);
void showTextWithMoney(wchar_t* str, int money);
void showText(wchar_t* str);
void highlightPlayer(int player);
void setSellLandContext(int priceNeed, int totalPrice);
int loolLandsaleMenu(int pos, int needPrice, int landown, int list1, int list2, int list3, int list4);
void highlightTileBlue(int index, bool highlight);
void payTax(int fee);
void getPrize(int money);

int getX(int blockindex);
int getY(int blockindex);
void initiate();
void insertTestData();

int LoopLandSaleMenu(int index, int buildingList);
int loopTravelMenu(int pos);
int checkUndertake(int index, int price);
int checkLoan(int index, int price);
void simpleMessage(wchar_t* msg, wchar_t* okayMessage);
int checkTravel(int price);
void yourTurn();

int escape(int remaining);
void reception(int money);
void welfare(int money);
void island(int remaining);
int blackcard();
void toll(int visitor, int owner, int price);

void mainLoop(int serv_socket);
void startGame(int player, int pcount);
void endGame();