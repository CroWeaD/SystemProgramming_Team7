#include "game.h"
#include "gamePacket.h"

Square squares[TOTAL_SQR] = {
    {3, 0, -1, {0,}, {0,}},                                                                      // 시작
    {0, 0, -1, {25*TUSD, 15*TUSD, 5*TUSD, 5*TUSD}, {25*TUSD, 9*TUSD, 1*TUSD, 0.2*TUSD}},         // 타이페이
    {2, 0, -1, {0,}, {0,}},                                                                      // 황금열쇠
    {0, 0, -1, {25*TUSD, 15*TUSD, 5*TUSD, 8*TUSD}, {45*TUSD, 18*TUSD, 2*TUSD, 0.4*TUSD}},        // 베이징
    {0, 0, -1, {25*TUSD, 15*TUSD, 5*TUSD, 8*TUSD}, {45*TUSD, 18*TUSD, 2*TUSD, 0.4*TUSD}},        // 마닐라
    {1, 14, -1, {0,0,0,20*TUSD}, {0,0,0,30*TUSD}},                                               // 제주도
    {0, 0, -1, {25*TUSD, 15*TUSD, 5*TUSD, 10*TUSD}, {55*TUSD, 27*TUSD, 3*TUSD, 0.6*TUSD}},       // 싱가포르
    {3, 0, -1, {0,}, {0,}},                                                                      // 무인도
    {0, 0, -1, {50*TUSD, 30*TUSD, 10*TUSD, 14*TUSD}, {75*TUSD, 45*TUSD, 5*TUSD, 1*TUSD}},        // 아테네
    {2, 0, -1, {0,}, {0,}},                                                                      // 황금열쇠
    {0, 0, -1, {50*TUSD, 30*TUSD, 10*TUSD, 16*TUSD}, {75*TUSD, 45*TUSD, 6*TUSD, 1*TUSD}},        // 코펜하겐
    {1, 14, -1, {0,0,0,20*TUSD}, {0,0,0,30*TUSD}},                                               // 콩코드 여객기
    {0, 0, -1, {50*TUSD, 30*TUSD, 10*TUSD, 18*TUSD}, {95*TUSD, 55*TUSD, 7*TUSD, 1.4*TUSD}},      // 베를린
    {0, 0, -1, {50*TUSD, 30*TUSD, 10*TUSD, 20*TUSD}, {100*TUSD, 60*TUSD, 8*TUSD, 1.6*TUSD}},     // 몬트리올
    {3, 0, -1, {0,}, {0,0,0,0}},                                                                 // 사회복지기금 
    {0, 0, -1, {75*TUSD, 45*TUSD, 15*TUSD, 24*TUSD}, {110*TUSD, 75*TUSD, 10*TUSD, 2*TUSD}},      // 상파울루
    {2, 0, -1, {0,}, {0,}},                                                                      // 황금열쇠
    {1, 14, -1, {0,0,0,50*TUSD}, {0,0,0,60*TUSD}},                                               // 부산
    {0, 0, -1, {75*TUSD, 45*TUSD, 15*TUSD, 26*TUSD}, {115*TUSD, 80*TUSD, 11*TUSD, 2.2*TUSD}},    // 하와이
    {1, 14, -1, {0,0,0,30*TUSD}, {0,0,0,25*TUSD}},                                               // 퀸 엘리자베스호
    {0, 0, -1, {75*TUSD, 45*TUSD, 15*TUSD, 26*TUSD}, {115*TUSD, 80*TUSD, 11*TUSD, 2.2*TUSD}},    // 마드리드
    {3, 0, -1, {0,}, {0,0,0,20*TUSD}},                                                           // 우주여행
    {0, 0, -1, {100*TUSD, 60*TUSD, 20*TUSD, 30*TUSD}, {127*TUSD, 90*TUSD, 13*TUSD, 2.6*TUSD}},   // 도쿄
    {1, 14, -1, {0,0,0,45*TUSD}, {0,0,0,30*TUSD}},                                               // 컬럼비아호
    {0, 0, -1, {100*TUSD, 60*TUSD, 20*TUSD, 35*TUSD}, {150*TUSD, 110*TUSD, 17*TUSD, 3.5*TUSD}},  // 런던
    {2, 0, -1, {0,}, {0,}},                                                                      // 황금열쇠
    {3, 0, -1, {0,}, {0,0,0,15*TUSD}},                                                           // 기부
    {1, 14, -1, {0,0,0,100*TUSD}, {0,0,0,200*TUSD}}                                              // 서울
};

Player players[4] = {
    {0, -1, -1, 0, false, false, false, false, false},
    {0, -1, -1, 0, false, false, false, false, false},
    {0, -1, -1, 0, false, false, false, false, false},
    {0, -1, -1, 0, false, false, false, false, false}
};

static int golden_keys[9] = {0, 1, 2, 3, 5, 6, 7, 8};
static int pivot = 0;

static int state;                  // 0: end, 1: continue
static int currPlayer;             // 현재 player
static int playerNum;              // player 수
static int social_fund;            // 사회복지기금

int clnt_sock[4];

void tempSocket() {
    int temp_port = 8080;       // 임시 PORT
     playerNum = 2;              // player 수
    int opt = 1;

    struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);  
    if(serv_sock == -1) {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(temp_port);

    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1) {
        perror("bind");
        exit(1);
    }
	
	if(listen(serv_sock, 5)==-1) {
        perror("bind");
        exit(1);
    }
	
	clnt_addr_size=sizeof(clnt_addr);  

    for(int i=0; i<playerNum; i++) {
        clnt_sock[i] = accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
        if(clnt_sock[i] == -1) {
            perror("accept");
        }
    }
}
//void main() {
//    tempSocket();
//    start_game(2, clnt_sock);
//}
void start_game(int pnum, int clnt[4]) {

    playerNum = pnum;
    for(int i=0; i<playerNum; i++)
        clnt_sock[i] = clnt[i];
        
    set_up(playerNum);

    char name0[10] = "messi", name1[10] = "ronaldo", name2[10] = "mbappe", name3[10]="haland";
    for(int i=0; i<playerNum; i++)
        sendName(playerNum, i, name0, name1, name2, name3);

    while(1) {
        // 턴 시작
        start_turn();

        // 턴 넘기기
        currPlayer = (currPlayer + 1) % playerNum;
        /*-------------------------------------------------------------------*/
        if(state==0)    break;
        sleep(1);
        /*-------------------------------------------------------------------*/
    }
}


void set_up(int num) {
    srand(time(NULL));
    state = 1;
    currPlayer = 0;
    for(int i=0; i<num; i++) {
        players[i].pos = START;
        players[i].cash = 660*TUSD;
    }
}

void start_turn() {
    int curr_pos;

    /*-------------------------------------------------------------------*/
    printf("\n[player:%d]\n", currPlayer);
    /*-------------------------------------------------------------------*/
    
    // 현재 player 위치 확인
    if(players[currPlayer].pos == ISLAND_SQR) {
        
        // 무인도일 경우
        /*-------------------------------------------------------------------*/
        printf("무인도 남은 턴 수:%d\n", players[currPlayer].island_remaining);
        /*-------------------------------------------------------------------*/
        if(players[currPlayer].island_remaining == 0) {          // 무인도 남은턴 수 == 0
            roll_dice(0);
            return;
        } else if(players[currPlayer].has_escape) {            // 무인도 탈출권 있으면     
            /*SEND PACKET*/
            sendIslandEscape(playerNum, currPlayer, players[currPlayer].has_escape, players[currPlayer].island_remaining);
            /*RECV PACKET*/
            //recvTF
            int use_escape = recvPack(playerNum, currPlayer);

            if(use_escape) {    // 사용
                printf("사용\n");
                players[currPlayer].has_escape = false;
                roll_dice(0);
                return;
            }
        }
        //미사용
        printf("무인도 탈출권 미사용\n");
        
        // 주사위 입력받음
        /*SEND PACKET*/
        sendIslandDiceWait(playerNum, currPlayer, players[currPlayer].island_remaining);
        /*RECV PACKET*/     
        recvPack(playerNum, currPlayer);

        int die1 = rand() % 6 + 1;
        int die2 = rand() % 6 + 1;

        printf("die1: %d, die2: %d\n", die1, die2);

        if(die1 == die2) {  // 더블일 경우

            curr_pos = move_player(die1 + die2);
            /*SEND PACKET*/
            sendIslandDiceResult(playerNum, currPlayer, die1, die2, players[currPlayer].pos);
            /*RECV PACKET*/
            recvPack(playerNum, currPlayer);

            // 도착한 칸 판단
            arrived_square(curr_pos);
        } else {
            /*SEND PACKET*/
            sendIslandDiceResult(playerNum, currPlayer, die1, die2, 0);
            /*RECV PACKET*/
            recvPack(playerNum, currPlayer);
            players[currPlayer].island_remaining -= 1;
        }
    } else if(players[currPlayer].pos == SPACE_TRAVEL_SQR && players[currPlayer].paid_for_sship) {

        // 우주여행을 경우
        /*SEND PACKET*/
        sendWhere2Travel(playerNum, currPlayer);
        /*RECV PACKET*/
        int travel = recvPack(playerNum, currPlayer);
        travel = travel > SPACE_TRAVEL_SQR ? travel - SPACE_TRAVEL_SQR : travel + TOTAL_SQR - SPACE_TRAVEL_SQR;
        curr_pos = move_player(travel);
        /*SEND PACKET*/
        sendTravelResult(playerNum, currPlayer, curr_pos, players[currPlayer].cash);
        /*RECV PACKET*/
        recvPack(playerNum, currPlayer);

        players[currPlayer].paid_for_sship = false;
        arrived_square(curr_pos);
    } else {
        // 일반적인 경우
        roll_dice(0);
    }
}
void roll_dice(int doubleNum) {

    int die1, die2;
    bool is_double = false;
    int curr_pos;

    /*SEND PACKET*/
    sendDiceWait(playerNum, currPlayer);
    /*RECV PACKET*/
    recvPack(playerNum, currPlayer);

    // 주사위 결과 받음
    die1 = rand() % 6 + 1;
    die2 = rand() % 6 + 1;

    printf("die1: %d, die2: %d\n", die1, die2);
    if(die1 == die2) {
        doubleNum += 1;
        is_double = true;
        printf("double!! %d 번째\n", doubleNum);
    }

    // 더블이 3번 연속 나올경우
    if(doubleNum >= 3) {
        players[currPlayer].pos = ISLAND_SQR;
        /*SEND PACKET*/
        sendDouble3Time(playerNum, currPlayer, ISLAND_SQR);
        /*RECV PACKET*/
        recvPack(playerNum, currPlayer);

        return;
    }
    curr_pos = move_player(die1 + die2);
    /*SEND PACKET*/
    sendDiceResult(playerNum, currPlayer, die1, die2, players[currPlayer].pos, doubleNum, players[currPlayer].cash);
    /*RECV PACKET*/
    recvPack(playerNum, currPlayer);
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
        arrived_golden_key(curr_pos);
    } else {
        arrived_etc();
    }
}

void arrived_city(int type, int curr_pos) {
    bool accessible = false;
    int host = squares[curr_pos].owner;

    if(squares[curr_pos].owner != currPlayer && squares[curr_pos].owner != -1) {       // 다른 사람의 도시일 경우
        printf("player %d의 도시 도착\n", squares[curr_pos].owner);

        if(players[currPlayer].has_voucher) {               // 우대권이 있다면

            /*SEND PACKET*/
            sendAskVoucher(playerNum, currPlayer);
            printf("우대권이 있습니다. 사용하시겠습니까?: ");
            /*RECV PACKET*/
            // 우대권 사용 여부 
             int use_voucher = recvPack(playerNum, currPlayer);

            // 우대권 사용할 경우
            if(use_voucher) {
                accessible = true;
                /*SEND PACKET*/
                sendUseVoucher(playerNum, currPlayer);
                /*RECV PACKET*/
                recvPack(playerNum, currPlayer);
                printf("사용\n");
            }
        }
        if(!accessible) {    // 우대권 사용 안했을 경우
            int total_toll = 0; 
            int owner_bitmask=0;
            int owned_buildings[4]= {0,0,0,0};
            printf("우대권 미사용\n");
            total_toll = cal_toll(curr_pos);
            if(total_toll<= players[currPlayer].cash) {    // 우대권을 안써도 돈이 있다면
                players[squares[curr_pos].owner].cash += total_toll;
                players[currPlayer].cash -= total_toll;
                accessible = true;
                /*SEND PACKET*/
                sendPayResult(playerNum, currPlayer, host, total_toll, 
                                players[currPlayer].cash, players[host].cash);
                /*RECV PACKET*/
                recvPack(playerNum, currPlayer);

                printf("지불한 비용: %d\n", total_toll);
            } else if(real(currPlayer)/2 + players[currPlayer].cash >= total_toll) {         // 영끌 가능할 경우
                if(!players[currPlayer].has_loan) {        // 대출한 적이 없으면
                    /*SEND PACKET*/
                    sendWillLoan(playerNum, currPlayer, curr_pos, host, total_toll);
                    /*RECV PACKET*/
                    // 대출 여부 받기
                    int loan = recvPack(playerNum, currPlayer);
                    if(loan) {
                        players[currPlayer].has_loan = true;
                        players[currPlayer].cash = 0;
                        players[squares[curr_pos].owner].cash += total_toll;
                        sendLoanMsg(playerNum, currPlayer);
                        printf("대출\n");
                    } else {
                        // 매각 토지 결정
                        for(int i=TOTAL_SQR-1, j=0; i>=0; i--) {
                            if( (i+1) % 8 == 0)  j++;
                            owner_bitmask = (owner_bitmask<<1) | (squares[i].owner==currPlayer ? 1 : 0);
                            owned_buildings[j] = (owned_buildings[j]<<4) | squares[i].buildings;
                        }
                        /*SEND PACKET*/
                        sendChooseLand2Sell(playerNum, currPlayer, curr_pos, total_toll, owner_bitmask, 
                                            owned_buildings[3], owned_buildings[2], owned_buildings[1], owned_buildings[0]);
                        /*RECV PACKET*/
                        int sell_bitmask=recvPack(playerNum, currPlayer);
                        checkBitsSet(owner_bitmask, sell_bitmask, currPlayer);
                        players[squares[curr_pos].owner].cash += total_toll;
                        players[currPlayer].cash -= total_toll;
                        if(players[currPlayer].cash<0) {
                            bankruptcy(currPlayer);
                            return;
                        }

                        /*SEND PACKET*/
                        sendLandSellResult(playerNum, currPlayer, sell_bitmask, players[currPlayer].cash);
                        /*RECV PACKET*/
                        recvPack(playerNum, currPlayer);
                    }
                }
            } else {                                    // 영끌 불가능할 경우
                if(!players[currPlayer].has_loan) {        // 대출한 적이 없으면
                    players[currPlayer].has_loan = true;
                    players[currPlayer].cash = 0;
                    players[squares[curr_pos].owner].cash += total_toll;
                    printf("대출\n");
                    sendLoanMsg(playerNum, currPlayer);
                    recvPack(playerNum, currPlayer);
                } else {

                    players[currPlayer].cash -= total_toll;
                    bankruptcy(currPlayer);
                    return;
                }
            }
        }
    }

    if(accessible) {                                                                              // 인수 가능
        int total_price = cal_price(curr_pos) *2;
        if(total_price <= players[currPlayer].cash) {    // 구매 가능
            /*SEND PACKET*/
            sendTakeOverWait(playerNum, currPlayer, curr_pos, total_price, squares[curr_pos].owner);
            /*RECV PACKET*/
            int take_over = recvPack(playerNum, currPlayer);
            if(take_over) {     // 인수 결정
                printf("인수\n");
                trading(curr_pos, total_price, squares[curr_pos].owner, currPlayer);
                /*SEND PACKET*/
                sendTakeOverResult(playerNum, currPlayer, curr_pos, host, total_price, 
                                players[currPlayer].cash, players[host].cash); 
                /*RECV PACKET*/
                recvPack(playerNum, currPlayer);
            }
        }
    }
    if(squares[curr_pos].owner == currPlayer || squares[curr_pos].owner == -1) {                    // 본인의 도시일 경우 or 주인 없는 도시일 경우
        printf("type: %d\n", type);
        printf("buildings: %d\n", squares[curr_pos].buildings);
        if(type == CITY) {              // 일반 도시일 경우
            if(squares[curr_pos].buildings != (B_HOTEL + B_BUILDING + B_VILLA + B_LAND)) {
                /*SEND PACKET*/
                sendChooseLand2Buy(playerNum, currPlayer, curr_pos, squares[curr_pos].buildings);
                /*RECV PACKET*/
                int receipt = recvPack(playerNum, currPlayer);
                int total_price = 0;
                // 건물 구입
                total_price += construction(receipt, currPlayer, curr_pos, LAND, B_LAND);
                total_price += construction(receipt, currPlayer, curr_pos, VILLA, B_VILLA);
                total_price += construction(receipt, currPlayer, curr_pos, BUILDING, B_BUILDING);
                total_price += construction(receipt, currPlayer, curr_pos, HOTEL, B_HOTEL);
                
                int changed_toll = cal_toll(curr_pos);
                /*SEND PACKET*/
                sendLandPuchaseResult(playerNum, currPlayer, curr_pos, total_price, squares[curr_pos].buildings, players[currPlayer].cash, changed_toll);
                /*RECV PACKET*/
                recvPack(playerNum, currPlayer);
            }
        } else if(type == SPECIAL) {    // 특수 도시일 경우
            if(squares[curr_pos].buildings != (B_LAND)) {
                int total_price = 0;
                /*SEND PACKET*/
                sendChooseLand2Buy(playerNum, currPlayer, curr_pos, squares[curr_pos].buildings);
                /*RECV PACKET*/
                // 건물 구입
                int receipt = recvPack(playerNum, currPlayer);
                total_price += construction(receipt, currPlayer, curr_pos, LAND, B_LAND);
                int changed_toll = cal_toll(curr_pos);
                /*SEND PACKET*/
                sendLandPuchaseResult(playerNum, currPlayer, curr_pos, total_price, squares[curr_pos].buildings, players[currPlayer].cash, changed_toll);
                /*RECV PACKET*/
                recvPack(playerNum, currPlayer);
            }
        }
    } 
}

void arrived_etc() {
    if(players[currPlayer].pos == ISLAND_SQR) {               // 무인도
        printf("무인도 도착\n");
        players[currPlayer].island_remaining = 2;
    }else if(players[currPlayer].pos == SOCIAL_FUND_SQR) {    // 사회복지기금
        printf("사회복지기금 수령\n");
        players[currPlayer].cash += social_fund;
        /*SEND PACKET*/
        sendReceivingFunds(playerNum, currPlayer, social_fund, players[currPlayer].cash);
        /*RECV PACKET*/
        recvPack(playerNum, currPlayer);
        social_fund = 0;
    }else if(players[currPlayer].pos == DONATION_SQR) {       // 기부
        printf("사회복지기금 기부\n");
        int donation = 0;
        if(players[currPlayer].cash >= squares[DONATION_SQR].toll[LAND]) 
            donation = squares[DONATION_SQR].toll[LAND];
        else 
            donation = players[currPlayer].cash;
        players[currPlayer].cash -= donation;
        social_fund += donation;
        /*SEND PACKET*/
        sendDonation(playerNum, currPlayer, donation, social_fund, players[currPlayer].cash);
        /*RECV PACKET*/
        recvPack(playerNum, currPlayer);
    }else if(players[currPlayer].pos == SPACE_TRAVEL_SQR) {   // 우주여행
        printf("우주여행 도착\n");
        if(squares[SPACE_TRAVEL_SQR].owner == -1 || (players[currPlayer].cash < squares[SPACE_TRAVEL_SQR].toll[LAND])) {
            /*SEND PACKET*/
            sendAskTravel(playerNum, currPlayer, squares[SPACESHIP].owner, squares[SPACESHIP].toll[LAND]);
            /*RECV PACKET*/
            int ride = recvPack(playerNum, currPlayer);
            if(ride) {
                printf("우주왕복선 탑승\n");
                players[currPlayer].cash                   -= squares[SPACE_TRAVEL_SQR].toll[LAND];
                players[squares[SPACE_TRAVEL_SQR].owner].cash += squares[SPACE_TRAVEL_SQR].toll[LAND];
                players[currPlayer].paid_for_sship = true;
                /*SEND PACKET*/
                sendPayResult(playerNum, currPlayer, squares[SPACESHIP].owner, squares[SPACE_TRAVEL_SQR].toll[LAND], 
                            (squares[SPACESHIP].owner != -1 ? players[squares[SPACESHIP].owner ].cash : -1), players[currPlayer].cash);
                /*RECV PACKET*/
                recvPack(playerNum, currPlayer);
            }
        }
    }
}
int move_player(int move) {
    int salary = SALARY;    // 봉급

    if(players[currPlayer].pos + move >= TOTAL_SQR) {        // 시작지점을 지날 경우
        players[currPlayer].cash += salary;
        printf("cash: %d\n", players[currPlayer].cash);
    }
    players[currPlayer].pos = (players[currPlayer].pos + move) % TOTAL_SQR;
    printf("position: %d\n", players[currPlayer].pos);
    return players[currPlayer].pos;
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
    /*SEND PACKET*/
    sendBankruptcy(playerNum, currPlayer, winner);
    /*RECV PACKET*/
    recvPack(playerNum, currPlayer);

    state = 0;
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

void checkBitsSet(int bitmask1, int bitmask2, int currPlayer) {
    int bitpos = 1;
    for(int i=0; i<32; i++) {
        if ((bitmask1 & (1 << i)) && (bitmask2 & (1 << i)))
            trading(i, cal_price(i)/2, currPlayer, -1);
    }
}

int construction(int receipt, int currPlayer, int curr_pos, int building_type,  int building_bit) {
    if((squares[curr_pos].buildings & building_bit) || !(receipt & building_bit) || (players[currPlayer].cash < squares[curr_pos].price[building_type]))
        return 0;
    squares[curr_pos].buildings |= building_bit;
    trading(curr_pos, squares[curr_pos].price[building_type], -1, currPlayer);
    return squares[curr_pos].price[building_type];
}

void arrived_golden_key(int curr_pos) {
    if(pivot >= 9)
        shuffle();
    int card = golden_keys[pivot++];
    int move;
    
    printf("golden key: %d\n", card);
    /*SEND PACKET*/
    switch(card) {
        case 0: // 우대권
            players[currPlayer].has_voucher = true;
            sendGoldenKey(playerNum, currPlayer, card, 0, 0, 0, 0, 0);
            break;
        case 1: // 무인도 탈출권  
            players[currPlayer].has_escape = true;
            sendGoldenKey(playerNum, currPlayer, card, 0, 0, 0, 0, 0);
            break;
        case 2: // 정기종합소득세
            int arr[3] = {0,};
            int t_tax = 0;
            for(int i=0; i<TOTAL_SQR; i++) 
                if(currPlayer == squares[i].owner)
                    memcpy(arr, tax(i, arr), sizeof(int) * 3);
            t_tax = arr[0] * 150 + arr[1] * 100 + arr[2] * 30;
            if(players[currPlayer].cash < t_tax)
                t_tax = players[currPlayer].cash;
            players[currPlayer].cash -= t_tax;
            sendGoldenKey(playerNum, currPlayer, card, t_tax, arr[0], arr[1], arr[2], players[currPlayer].cash);
            break;
        case 3: // 관광여행(제주)
            int jeju = 5;
            move = curr_pos > jeju ? TOTAL_SQR - curr_pos + jeju : jeju - curr_pos;
            curr_pos = jeju;
            move_player(move);
            sendGoldenKey(playerNum, currPlayer, card, jeju, players[currPlayer].cash, 0, 0, 0);
            break;
        case 4: // 관광여행(부산)
            int busan = 17;
            move = curr_pos > busan ? TOTAL_SQR - curr_pos + busan : busan - curr_pos;
            curr_pos = busan;
            move_player(move);
            sendGoldenKey(playerNum, currPlayer, card, busan, players[currPlayer].cash, 0, 0, 0);
            break;
        case 5: // 관광여행(서울)
            int seoul = 27;
            move = curr_pos > seoul ? TOTAL_SQR - curr_pos + seoul : seoul - curr_pos;
            curr_pos = seoul;
            move_player(move);
            sendGoldenKey(playerNum, currPlayer, card, seoul, players[currPlayer].cash, 0, 0, 0);
            break;
        case 6: // 무인도 이동
            players[currPlayer].pos = ISLAND_SQR;
            sendGoldenKey(playerNum, currPlayer, card, ISLAND_SQR, 0, 0, 0, 0);
            break;
        case 7: // 우주여행 초대권
            move = curr_pos > SPACE_TRAVEL_SQR ? TOTAL_SQR - curr_pos + SPACE_TRAVEL_SQR : SPACE_TRAVEL_SQR - curr_pos;    
            move_player(move);
            players[currPlayer].paid_for_sship = true;
            sendGoldenKey(playerNum, currPlayer, card, SPACE_TRAVEL_SQR, players[currPlayer].cash, 0, 0, 0);        
            break;
        case 8: // 노벨상 500
            players[currPlayer].cash += 500;
            sendGoldenKey(playerNum, currPlayer, card, 500, players[currPlayer].cash, 0, 0, 0);
            break;
    }
    /*RECV PACKET*/
    recvPack(playerNum, currPlayer);
    if(card==3 || card==4 || card==5) 
        arrived_square(players[currPlayer].pos);
}

int* tax(int curr_pos, int* arr) {
    if(squares[curr_pos].buildings & B_HOTEL)     arr[0]++;
    if(squares[curr_pos].buildings & B_BUILDING)  arr[1]++;
    if(squares[curr_pos].buildings & B_VILLA)     arr[2]++;  
    return arr;
}

void shuffle() {
    int idx, temp;
    for(int i=0; i<9; i++) {
        idx = rand() % 9;
        temp = golden_keys[idx];
        golden_keys[idx] = golden_keys[i];
        golden_keys[i] = temp;
    }
    pivot = 0;
}