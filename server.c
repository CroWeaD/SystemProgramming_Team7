#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define CITY              0         // 도시
#define SPECIAL           1         // 특수
#define GOLDED_KEY        2         // 황금열쇠

#define START             0
#define ISLAND_SQR       10         // 무인도 칸
#define SOCIAL_FUND_SQR  20         // 사회기금 칸
#define SPACE_TRAVEL_SQR 30         // 우주여행 칸
#define DONATION_SQR     38         // 기부 칸
#define TOTAL_SQR        40         // 전체 칸

#define MAN           10000
#define SALARY       200000

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
    int island_countdown;       // 무인도 남은 턴 수
    bool has_loan;              // 대출 여부
    bool has_voucher;           // 우대권 소유 여부
    bool has_escape;            // 무인도 탈출권 소유 여부
    bool paid_for_sship;        // 우주왕복선 비용 지불 여부
    bool passed_startpoint;     // 시작점 지났는지 여부
} Player;


Square squares[40] = {
    {3, 0, -1, {0,}, {0,}},                                                              // 시작
    {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 5*MAN}, {25*MAN, 9*MAN, 1*MAN, 0.2*MAN}},         // 타이페이
    {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
    {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 8*MAN}, {45*MAN, 18*MAN, 2*MAN, 0.4*MAN}},        // 홍콩
    {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 8*MAN}, {45*MAN, 18*MAN, 2*MAN, 0.4*MAN}},        // 마닐라
    {1, 0, -1, {0,0,0,20*MAN}, {0,0,0,30*MAN}},                                          // 제주도
    {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 10*MAN}, {55*MAN, 27*MAN, 3*MAN, 0.6*MAN}},       // 싱가포르
    {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
    {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 10*MAN}, {55*MAN, 27*MAN, 3*MAN, 0.6*MAN}},       // 카이로
    {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 12*MAN}, {60*MAN, 30*MAN, 4*MAN, 0.8*MAN}},       // 이스탄불
    {3, 0, -1, {0,}, {0,}},                                                              // 무인도
    {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 14*MAN}, {75*MAN, 45*MAN, 5*MAN, 1*MAN}},        // 아테네
    {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
    {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 16*MAN}, {75*MAN, 45*MAN, 6*MAN, 1*MAN}},        // 코펜하겐
    {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 16*MAN}, {75*MAN, 45*MAN, 6*MAN, 1*MAN}},        // 스톡홀름
    {1, 0, -1, {0,0,0,20*MAN}, {0,0,0,30*MAN}},                                          // 콩코드 여객기
    {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 18*MAN}, {95*MAN, 55*MAN, 7*MAN, 1.4*MAN}},      // 취리히
    {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
    {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 16*MAN}, {90*MAN, 50*MAN, 6*MAN, 1.2*MAN}},      // 베를린
    {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 20*MAN}, {100*MAN, 60*MAN, 8*MAN, 1.6*MAN}},     // 몬트리올
    {3, 0, -1, {0,}, {0,0,0,0}},                                                         // 사회복지기금 
    {0, 0, -1, {75*MAN, 40*MAN, 15*MAN, 22*MAN}, {105*MAN, 70*MAN, 9*MAN, 1.8*MAN}},     // 부에노스아이레스
    {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
    {0, 0, -1, {75*MAN, 45*MAN, 15*MAN, 24*MAN}, {110*MAN, 75*MAN, 10*MAN, 2*MAN}},      // 상파울루
    {0, 0, -1, {75*MAN, 45*MAN, 15*MAN, 24*MAN}, {110*MAN, 75*MAN, 10*MAN, 2*MAN}},      // 시드니
    {1, 0, -1, {0,0,0,50*MAN}, {0,0,0,60*MAN}},                                          // 부산
    {0, 0, -1, {75*MAN, 45*MAN, 15*MAN, 26*MAN}, {115*MAN, 80*MAN, 11*MAN, 2.2*MAN}},    // 하와이
    {0, 0, -1, {75*MAN, 45*MAN, 15*MAN, 26*MAN}, {115*MAN, 80*MAN, 11*MAN, 2.2*MAN}},    // 리스본
    {1, 0, -1, {0,0,0,30*MAN}, {0,0,0,25*MAN}},                                          // 퀸 엘리자베스호
    {0, 0, -1, {75*MAN, 45*MAN, 15*MAN, 28*MAN}, {120*MAN, 85*MAN, 12*MAN, 2.4*MAN}},    // 마드리드
    {3, 0, -1, {0,}, {0,0,0,20*MAN}},                                                    // 우주여행
    {0, 0, -1, {100*MAN, 60*MAN, 20*MAN, 30*MAN}, {127*MAN, 90*MAN, 13*MAN, 2.6*MAN}},   // 도쿄
    {1, 0, -1, {0,0,0,45*MAN}, {0,0,0,30*MAN}},                                          // 콜롬비아호
    {0, 0, -1, {100*MAN, 60*MAN, 20*MAN, 32*MAN}, {140*MAN, 100*MAN, 15*MAN, 2.8*MAN}},  // 파리
    {0, 0, -1, {100*MAN, 60*MAN, 20*MAN, 32*MAN}, {140*MAN, 100*MAN, 15*MAN, 2.8*MAN}},  // 로마
    {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
    {0, 0, -1, {100*MAN, 60*MAN, 20*MAN, 35*MAN}, {150*MAN, 110*MAN, 17*MAN, 3.5*MAN}},  // 런던
    {0, 0, -1, {100*MAN, 60*MAN, 20*MAN, 35*MAN}, {150*MAN, 110*MAN, 17*MAN, 3.5*MAN}},  // 뉴욕
    {3, 0, -1, {0,}, {0,0,0,15*MAN}},                                                    // 기부
    {1, 0, -1, {0,0,0,100*MAN}, {0,0,0,200*MAN}}                                         // 서울
};

Player players[4] = {
    {0, 330*MAN, START, 0, false, false, false, false, false},
    {0, 330*MAN, START, 0, false, false, false, false, false},
    {0, 330*MAN, START, 0, false, false, false, false, false},
    {0, 330*MAN, START, 0, false, false, false, false, false}
};


int state;                      // ?
int currentPlayer;              // 현재 player
int playerNum;                  // player 수
int social_fund;                // 사회복지기금

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

int main(int argc, char* argv[]) {
    
    set_up(2);

    while(1) {
        // 턴 시작
        start_turn();

        // 턴 넘기기
        currentPlayer = (currentPlayer + 1) % playerNum;
        /*-------------------------------------------------------------------*/
        sleep(1);
        /*-------------------------------------------------------------------*/
    }
}


void set_up(int num) {
    srand(time(NULL));
    currentPlayer = 0;
    playerNum = num;
    if(num == 2) {
        for(int i=0; i<2; i++)
            players[i].cash *= 2;
    }
}

void start_turn() {
    int curr_pos;

    printf("\n[player:%d]\n", currentPlayer);
    // 현재 player 위치 확인
    if(players[currentPlayer].pos == ISLAND_SQR) {
        
        // 무인도일 경우
        /*-------------------------------------------------------------------*/
        printf("무인도 남은 턴 수:%d\n", players[currentPlayer].island_countdown);
        /*-------------------------------------------------------------------*/
        if(players[currentPlayer].island_countdown == 0) {          // 무인도 남은턴 수 == 0
            roll_dice(0);
            return;
        }
        else if(players[currentPlayer].has_escape) {            // 무인도 탈출권 있으면            
            /*-------------------------------------------------------------------*/
            // use_escape 받음
            bool use_escape = false;
            printf("무인도 탈출권이 있습니다. 사용하시겠습니까?: ");
            /*-------------------------------------------------------------------*/

            if(use_escape) {    // 사용
                printf("사용\n");
                players[currentPlayer].has_escape = false;
                roll_dice(0);
                return;
            }
        }
        //미사용
        printf("무인도 탈출권 미사용\n");
        /*-------------------------------------------------------------------*/
        // 주사위 입력받음
        int die1 = rand() % 6 + 1;
        int die2 = rand() % 6 + 1;
        /*-------------------------------------------------------------------*/

        printf("die1: %d, die2: %d\n", die1, die2);

        if(die1 == die2) {  // 더블일 경우
            curr_pos = move_player(die1 + die2);
            // 도착한 칸 판단
            arrived_square(curr_pos);
            // 더블 판단
            if(!(players[currentPlayer].pos == ISLAND_SQR || players[currentPlayer].pos == SPACE_TRAVEL_SQR)) {
                roll_dice(1);
            }
        } else {
            players[currentPlayer].island_countdown -= 1;
        }
        
    }
    else if(players[currentPlayer].pos == SPACE_TRAVEL_SQR && players[currentPlayer].paid_for_sship) {

        // 우주여행을 경우
        /*-------------------------------------------------------------------*/
        // 여행할 칸 선택
        int travel = rand() % TOTAL_SQR;
        /*-------------------------------------------------------------------*/
        move_player(travel);
        players[currentPlayer].paid_for_sship = false;
    } else {

        // 일반적인 경우
        roll_dice(0);
    }
}
void roll_dice(int doubleNum) {

    int die1, die2;
    bool is_double = false;
    int curr_pos;

    /*-------------------------------------------------------------------*/
    // 주사위 결과 받음
    die1 = rand() % 6 + 1;
    die2 = rand() % 6 + 1;
    /*-------------------------------------------------------------------*/

    
    printf("die1: %d, die2: %d\n", die1, die2);
    if(die1 == die2) {
        doubleNum += 1;
        is_double = true;
        printf("double!! %d 번째\n", doubleNum);
    }

    // 더블이 3번 연속 나올경우
    if(doubleNum >= 3) {
        players[currentPlayer].pos = ISLAND_SQR;
        return;
    }
    curr_pos = move_player(die1 + die2);
    // 도착한 칸 판단
    arrived_square(curr_pos);

    // 더블 판단
    if(is_double && !(curr_pos == ISLAND_SQR || curr_pos == SPACE_TRAVEL_SQR)) {
        roll_dice(doubleNum);
    }

}

void arrived_square(int curr_pos) {
    // 도착한 칸이 도시일 경우
    if(squares[curr_pos].type == CITY) {               // 도시일 경우
        printf("도시 도착\n");
        arrived_city(CITY, curr_pos);                        
    }
    else if(squares[curr_pos].type == SPECIAL) {       // 특수도시일 경우
        printf("특수도시 도착\n");
        arrived_city(SPECIAL, curr_pos);
    }
    else if(squares[curr_pos].type == GOLDED_KEY) {    // 황금열쇠일 경우
        // 황금열쇠를 뽑는 함수
        printf("GOLDEN KEY!!\n");
    } else {
        arrived_etc();
    }
}

void arrived_city(int type, int curr_pos) {
    bool accessible = false;
    int total_price = 0;
    int total_toll = 0; 
    if(squares[curr_pos].owner != currentPlayer && squares[curr_pos].owner != -1) {       // 다른 사람의 도시일 경우
        printf("player %d의 도시 도착\n", squares[curr_pos].owner);
        if(players[currentPlayer].has_voucher) {               // 우대권이 있다면
            printf("우대권이 있습니다. 사용하시겠습니까?: ");
            /*-------------------------------------------------------------------*/
            //우대권 사용여부
            bool use_voucher = true;
            /*-------------------------------------------------------------------*/
            // 우대권 사용할 경우
            if(use_voucher) {
                accessible = true;
                printf("사용\n");
            }
        }
        if(!accessible) {    // 우대권 사용 안했을 경우
            printf("우대권 미사용\n");
            total_toll = cal_toll(curr_pos);
            if(total_toll<= players[currentPlayer].cash) {    // 우대권을 안써도 돈이 있다면
                players[squares[curr_pos].owner].cash += total_toll;
                players[currentPlayer].cash -= total_toll;
                accessible = true;
                printf("지불한 비용: %d\n", total_toll);
            } else if(real(currentPlayer)/2 + players[currentPlayer].cash >= total_toll) {         // 영끌 가능할 경우
                if(!players[currentPlayer].has_loan) {        // 대출한 적이 없으면
                    /*-------------------------------------------------------------------*/
                    // 대출권 사용 할지
                    int loan = true;
                    /*-------------------------------------------------------------------*/
                    if(loan) {
                        players[currentPlayer].has_loan = true;
                        players[currentPlayer].cash = 0;
                        players[squares[curr_pos].owner].cash += total_toll;
                        printf("대출\n");
                    } else {
                        /*-------------------------------------------------------------------*/
                        // 매각 토지 결정

                        for(int i=0; i<TOTAL_SQR; i++) {
                            if(squares[i].owner==currentPlayer) {
                                trading(i, cal_price(i)/2, currentPlayer, -1);
                            }
                        }
                        players[squares[curr_pos].owner].cash += total_toll;
                        players[currentPlayer].cash -= total_toll;

                        /*-------------------------------------------------------------------*/
                    }
                } else {                                    // 영끌 불가능할 경우
                    if(!players[currentPlayer].has_loan) {        // 대출한 적이 없으면
                        players[currentPlayer].has_loan = true;
                        players[currentPlayer].cash = 0;
                        players[squares[curr_pos].owner].cash += total_toll;
                        printf("대출\n");
                    } else {
                        players[currentPlayer].cash -= total_toll;
                        bankruptcy(currentPlayer);
                    }
                }
            }
        }
    }

    if(accessible) {                                                                              // 인수 가능
        total_price = cal_price(curr_pos) *2;
        if(total_price <= players[currentPlayer].cash) {    // 구매 가능
            /*-------------------------------------------------------------------*/
            printf("인수 하시겠습니까?: ");
            // 구매여부 
            bool take_over = true;
            /*-------------------------------------------------------------------*/
            if(take_over) {     // 인수 결정
                printf("인수\n");
                trading(curr_pos, total_price, squares[curr_pos].owner, currentPlayer); 
            }
        }
    }
int temp = 0;
    if(squares[curr_pos].owner == currentPlayer || squares[curr_pos].owner == -1) {                    // 본인의 도시일 경우 or 주인 없는 도시일 경우
        printf("type: %d\n", type);
        printf("buildings: %d\n", squares[curr_pos].buildings);
        if(type == CITY) {              // 일반 도시일 경우
            if(squares[curr_pos].buildings != (HOTEL + BUILDING + VILLA + LAND)) {
            /*-------------------------------------------------------------------*/
            // 뭐살지 결정
            // 건물 구입
            if(!(squares[curr_pos].buildings & B_LAND) && players[currentPlayer].cash >= squares[curr_pos].price[LAND]) {
                squares[curr_pos].buildings |= B_LAND;
                trading(curr_pos, squares[curr_pos].price[LAND], -1, currentPlayer);
                temp += squares[curr_pos].price[LAND];
            }
            if(!(squares[curr_pos].buildings & B_VILLA) && players[currentPlayer].cash >= squares[curr_pos].price[VILLA]) {
                squares[curr_pos].buildings |= B_VILLA;
                trading(curr_pos, squares[curr_pos].price[VILLA], -1, currentPlayer);
                temp += squares[curr_pos].price[VILLA];
            }
            if(!(squares[curr_pos].buildings & B_BUILDING) && players[currentPlayer].cash >= squares[curr_pos].price[BUILDING]) {
                squares[curr_pos].buildings |= B_BUILDING;
                trading(curr_pos, squares[curr_pos].price[BUILDING], -1, currentPlayer);
                temp += squares[curr_pos].price[BUILDING];
            }
            if(!(squares[curr_pos].buildings & B_HOTEL) && players[currentPlayer].cash >= squares[curr_pos].price[HOTEL]) {
                squares[curr_pos].buildings |= B_HOTEL;
                trading(curr_pos, squares[curr_pos].price[HOTEL], -1, currentPlayer);
                temp += squares[curr_pos].price[HOTEL];
            }
            
            /*-------------------------------------------------------------------*/
            }
        } else if(type == SPECIAL) {    // 특수 도시일 경우
            if(squares[curr_pos].buildings != (LAND)) {
                /*-------------------------------------------------------------------*/
                // 뭐살지 결정
                // 건물 구입
                if(!(squares[curr_pos].buildings & B_LAND) && players[currentPlayer].cash >= squares[curr_pos].price[LAND]) {
                    squares[curr_pos].buildings |= B_LAND;
                    trading(curr_pos, squares[curr_pos].price[LAND], -1, currentPlayer);
                    temp += squares[curr_pos].price[LAND];
                }
                /*-------------------------------------------------------------------*/
            }
        }
    } 

    /*-------------------------------------------------------------------*/
    // 결과 전달
    for(int i=0; i<playerNum; i++) {
        printf("건물 비용: %d\n", temp);
        printf("player %d cash: %d  ", i, players[i].cash);
    }
    printf("\n");
    /*-------------------------------------------------------------------*/
}

void arrived_etc() {
    if(players[currentPlayer].pos == ISLAND_SQR) {               // 무인도
        printf("무인도 도착\n");
        players[currentPlayer].island_countdown = 2;
    }else if(players[currentPlayer].pos == SOCIAL_FUND_SQR) {    // 사회복지기금
        printf("사회복지기금 수령\n");
        players[currentPlayer].cash += social_fund;
        social_fund = 0;
    }else if(players[currentPlayer].pos == DONATION_SQR) {       // 기부
        printf("사회복지기금 기부\n");
        if(players[currentPlayer].cash >= squares[DONATION_SQR].toll[LAND]) {
            players[currentPlayer].cash -= squares[DONATION_SQR].toll[LAND];
            social_fund += squares[DONATION_SQR].toll[LAND];
        } 
    }else if(players[currentPlayer].pos == SPACE_TRAVEL_SQR) {   // 우주여행
        printf("우주여행 도착\n");
        /*-------------------------------------------------------------------*/
        //우주왕복선을 탈지 선택
        bool ride = true;
        /*-------------------------------------------------------------------*/
        if(ride) {
            printf("우주왕복선 탑승\n");
            if(squares[SPACE_TRAVEL_SQR].owner != -1) {
                if(players[currentPlayer].cash < squares[SPACE_TRAVEL_SQR].toll[LAND]) {
                    printf("요금 부족\n");
                    return;
                }
                players[currentPlayer].cash                   -= squares[SPACE_TRAVEL_SQR].toll[LAND];
                players[squares[SPACE_TRAVEL_SQR].owner].cash += squares[SPACE_TRAVEL_SQR].toll[LAND];
                
            }
            players[currentPlayer].paid_for_sship = true;
        }
    }

}
int move_player(int move) {
    int salary = SALARY;    // 봉급

    if(players[currentPlayer].pos + move >= TOTAL_SQR) {        // 시작지점을 지날 경우
        players[currentPlayer].cash += salary;
        printf("cash: %d\n", players[currentPlayer].cash);
    }
    players[currentPlayer].pos = (players[currentPlayer].pos + move) % TOTAL_SQR;
    printf("position: %d\n", players[currentPlayer].pos);

    return players[currentPlayer].pos;
}

int real(int p) {
    int proprty = 0;
    for(int i=0; i<TOTAL_SQR; i++) {
        if(squares[i].owner == p) {
            proprty += cal_price(i);
        }
    }
    return proprty;
}

int cal_price(int curr_pos) {
    int total_price = 0;
    if(squares[curr_pos].buildings & B_HOTEL)     total_price += squares[curr_pos].price[HOTEL];
    if(squares[curr_pos].buildings & B_BUILDING)  total_price += squares[curr_pos].price[BUILDING];
    if(squares[curr_pos].buildings & B_VILLA)     total_price += squares[curr_pos].price[VILLA];
    if(squares[curr_pos].buildings & B_LAND)      total_price += squares[curr_pos].price[LAND];

    return total_price;
}

int cal_toll(int curr_pos) {
    int total_toll = 0;
    if(squares[curr_pos].buildings & B_HOTEL)     total_toll += squares[curr_pos].toll[HOTEL];
    if(squares[curr_pos].buildings & B_BUILDING)  total_toll += squares[curr_pos].toll[BUILDING];
    if(squares[curr_pos].buildings & B_VILLA)     total_toll += squares[curr_pos].toll[VILLA];
    if(squares[curr_pos].buildings & B_LAND)      total_toll += squares[curr_pos].toll[LAND];

    return total_toll;
}

void bankruptcy(int p) {
    int winner = -1;
    int property = 0;
    printf("player %d bankruptcy\n", p);
    for(int i=0; i<playerNum; i++) {
        if(real(i)+players[i].cash > property) {
            winner = i;
            property = real(i)+players[i].cash;
        }
    }
    printf("winner: %d\n", winner);
    exit(1);
}

void trading(int curr_pos, int total_price, int seller, int buyer) {
    if(seller == -1) {  // 주인 없는 도시 구매
        players[buyer].cash -= total_price;
        squares[curr_pos].owner = buyer;
    } else if(buyer == -1) {    // 은행에 판매
        players[seller].cash += total_price;
        squares[curr_pos].owner = -1;
        squares[curr_pos].buildings = 0;
    } else {
        players[buyer].cash -= total_price;
        players[seller].cash += total_price;
        squares[curr_pos].owner = buyer;
    }
    printf("%d 도시 판매(%d) %d->%d\n", curr_pos, total_price, seller, buyer);
}