#include "game_clnt.h"

int currentPlayer = 0;
int w = 8;
int h = 8;


/*
return type :
0 : ack
1 : true / false
2 : building list
3 : selling list
4 : travel place
*/
bool gameAction(int player, int* data, int type, SEND_PACKET* packet){
    packet->type = 0;
    int result1;
    int result2;
    clearContext();
    if (player == currentPlayer){
        switch(type){
            case 0:
                travel_move(data[0],data[1],data[2]);
                break;
            case 1:
                diceRoll(player);
                break;
            case 2:
                rollDice(player,data[0],data[1],data[2],data[4]);
                break;
            case 4:
                packet->type = 2;
                result1 = buyBuilding(data[0],player,data[1]);
                packet->data[0] = result1;
                break;
            case 5:
                packet->type = 1;
                result1 = ask_undertake(player,data[0],data[1],data[2]);
                packet->data[0] = result1;
                break;
            case 6:
                undertake_result(data[0],data[1],data[2],data[3],data[4],data[5]);
                break;
            case 7:
                payToll(data[0],data[1],data[2],data[3],data[4]);
                break;
            case 8:
                packet->type = 1;
                result1 = ask_loan(data[0],data[1],data[2]);
                packet->data[0] = result1;
                break;
            case 9:
                loan_result();
                break;
            case 10:
                landsale(0,0,&data[2]);
                break;
            case 11:
                packet->type = 1;
                result1 = ask_travel((currentPlayer==data[0]),data[1],data[2]);
                packet->data[0] = result1;
                break;
            case 12:
                packet->type = 4;
                result1 = travel(player);
                packet->data[0] = result1;
                break;
            case 13:
                packet->type = 1;
                result1 = ask_escape(data[1]);
                packet->data[0] = result1;
                break;
            case 14:
                roll_island();
                break;
            case 15:
                roll_island_result(player,data[0],data[1],data[2]);
                break;
            case 16:
                get_welfare(data[1],data[0],data[2]);
                break;
            case 17:
                pay_welfare(data[1],data[0],data[3]);
                break;
            case 19:
                packet->type = 1;
                result1 =  ask_blackcard(player);
                packet->data[0] = result1;
                break;
            case 20:
                result1 =  blackcard_result(player);
                break;
            case 21:
                buliding_buy_result(data[0],data[1],data[3],data[4],data[5]);
                break;

        }
        return true;
    }
    else{
        switch(type){
            case 0:
                travel_move(data[0],data[1],data[2]);
                break;
            case 1:
                diceRoll_2(player);
                break;
            case 2:
                rollDice(player,data[0],data[1],data[2],data[4]);
                break;
            case 4:
                buyBuilding_2(player);
                break;
            case 5:
                ask_undertake_2(player);
                break;
            case 6:
                undertake_result_2(data[0],data[1],data[2],data[3],data[4],data[5]);
                break;
            case 7:
                payToll_2(data[0],data[1],data[2],data[3],data[4]);
                break;
            case 8:
                ask_loan_2(player);
                break;
            case 9:
                loan_result_2(player);
                break;
            case 10:
                landsale(0,0,&data[2]);
                break;
            case 11:
                ask_travel_2(player);
                break;
            case 12:
                travel_2(player);
                break;
            case 13:
                ask_escape_2(data[1]);
                break;
            case 14:
                diceRoll_2(player);
                break;
            case 15:
                roll_island_result(player,data[0],data[1],data[2]);
                break;
            case 16:
                get_welfare_2(player,data[1],data[0],data[2]);
                break;
            case 17:
                pay_welfare_2(player,data[1],data[0],data[3]);
                break;
            case 19:
                ask_blackcard_1(player);
                break;
            case 20:
                blackcard_result_2(player);
                break;
            case 21:
                buliding_buy_result(data[0],data[1],data[3],data[4],data[5]);
                break;
        }
        refresh();
        return false;
    }
}

// 0. 싱크
void travel_move(int player, int index, int money){
    movePlayer(player,index);
    players[player].budget = money;
    highlightPlayer(player);
    drawPlayerInfo(player);
    refresh();
    usleep(300000);
}

// 1. 플레이어 주사위 대기
void diceRoll(int player){
    highlightPlayer(player);
    yourTurn();
}

void diceRoll_2(int player){
    clearContext();
    highlightPlayer(player);
    setHeader(L"다른 플레이어의 턴입니다.", true, COLOR_PAIR_DEAFULT);
    drawDiceBoard(COLOR_PAIR_DEAFULT);
}

// 2. 플레이어 주사위 결과
void rollDice(int player, int die1, int die2, int moving, int money){
    int sum = die1 + die2;
    wchar_t text[15];
    if(die1 == die2){
        swprintf(text,15,L"%d! 더블입니다.\0",sum);
    }
    else{
        swprintf(text,15,L"%d!\0",sum);
    }
    players[player].budget = money;
    setHeader(text, true, COLOR_PAIR_DEAFULT);
    drawDiceBoard(COLOR_PAIR_DEAFULT);
    drawDice(die1,die2,COLOR_PAIR_DEAFULT);
    refresh();
    usleep(1000000);
    movePlayer(player, moving);
    highlightPlayer(player);
    drawPlayerInfo(player);
    refresh();
    usleep(300000);
}

// 3. 플레이어 주사위 3 연속 더블
void tripleDouble(int player, int die1, int die2, int moving){
    drawDiceBoard(COLOR_PAIR_DEAFULT);
    drawDice(die1,die2,COLOR_PAIR_DEAFULT);
    
    setHeader(L"3 연속 더블! 무인도 칸으로 이동합니다.", true, COLOR_PAIR_DEAFULT);
    movePlayer(player, 7);
    highlightPlayer(player);
}

// 4. 플레이어 토지 구매
int buyBuilding(int index, int playerID, int buildingList){
        highlightTile(index,true);
        int result = LoopLandSaleMenu(index,buildingList);
        return result;
}

void buyBuilding_2(int playerID){
    otherPlayerMessage(playerID, L"다른 플레이어가 건물을 구매중입니다.",L"건물 매입");
}

// 5. 플레이어 인수 여부 확인
int ask_undertake(int playerID, int index, int price, int ownerID){
    highlightTile(players[playerID].position,true);
    if(checkUndertake(index, price)){
        clearContext();
        setHeader("",false,COLOR_PAIR_DEAFULT);
        return 1;
    }
    clearContext();
    setHeader("",false,COLOR_PAIR_DEAFULT);
    return 0;
}

void ask_undertake_2(int playerID){
    otherPlayerMessage(playerID, L"다른 플레이어가 인수 결정중입니다.",L"토지 인수");
}

// 6. 플레이어 인수 결과
void undertake_result(int index, int buyer, int seller, int price, int buyer_bud, int seller_bud){
        owner[index] = buyer;
        drawTile(index);
        setHeader(L"건물 인수",true,COLOR_PAIR_DEAFULT);
        showTextWithMoney(L"건물을 인수했습니다.",price);
        
        drawPlayerInfo(buyer);
        drawPlayerInfo(seller);            
}

void undertake_result_2(int index, int buyer, int seller, int price, int buyer_bud, int seller_bud){
        owner[index] = buyer;
        drawTile(index);
        otherPlayerMessage(buyer, L"건물을 인수했습니다.",L"건물 매입");

        drawPlayerInfo(buyer);
        drawPlayerInfo(seller);       
}

// 7. 요금 지불 결과
void payToll(int visitor, int owner, int price, int visitor_bud, int owner_bud){
    if(owner == -1){
        setTollContext(L"은행원",names[players[visitor].position],price);
    }
    else
        setTollContext(players[owner].name,names[players[visitor].position],price);
    setOkayButton(L"확인",COLOR_PAIR_HIGHLIGHT);
    players[visitor].budget = visitor_bud;
    players[owner].budget = owner_bud;
    drawPlayerInfo(visitor);
    drawPlayerInfo(owner);
}

void payToll_2(int visitor, int owner, int price, int visitor_bud, int owner_bud){
    otherPlayerMessage(visitor, L"다른 플레이어가 통행료를 지불합니다.",L"통행료 지불");
    players[visitor].budget = visitor_bud;
    players[owner].budget = owner_bud;
    drawPlayerInfo(visitor);
    drawPlayerInfo(owner);
}

// 8. 대출 여부 질문
int ask_loan(int index, int owner, int price){
    if(checkLoan(index,price))
        return 1;
    return 0;
}

void ask_loan_2(int player){
    otherPlayerMessage(player, L"다른 플레이어가 대출 여부를 결정중입니다.",L"대출");
}

// 9. 대출 확인 메시지
void loan_result(){
    simpleMessage(L"대출하였습니다.",L"확인");
}

void loan_result_2(int player){
    otherPlayerMessage(player, L"대출하였습니다.",L"대출");
}

// 10. 판매 토지 선택
void landsale(int index, int price, int* landList){

}

// 11. 세계여행 여부 선택
int ask_travel(int hasSpaceship, int playerID, int price){
    if(checkTravel(price))
        return 1;
    return 0;
}
void ask_travel_2(int playerID){
    otherPlayerMessage(playerID, L"다른 플레이어가 우주여행 여부를 결정중입니다.",L"우주여행");
}

// 12. 여행할 지역 선택
int travel(int playerID){
    int result = loopTravelMenu(players[playerID].position);
    setHeader("",false,COLOR_PAIR_DEAFULT);
    return result;
}

void travel_2(int playerID){
    otherPlayerMessage(playerID, L"다른 플레이어가 우주여행 목적지를 결정중입니다.",L"우주여행");
}


// 13. 무인도 탈줄권 사용 여부 선택
int ask_escape(int remaining){
    return escape(remaining);    
}

void ask_escape_2(int playerID){
    otherPlayerMessage(playerID,L"다른 플레이어가 무인도 탈줄권 사용 여부를 결정중입니다.",L"무인도");
}

// 14. 무인도 주사위 대기
void roll_island(){
    island(0);
}

// 15. 무인도 주사위 결과
void roll_island_result(int player, int die1, int die2, int moving){
    int sum = die1 + die2;
    wchar_t text[15];
    if(die1 == die2){
        swprintf(text,15,L"%d! 탈출했습니다.\0",sum);
    }
    else{
        swprintf(text,15,L"%d.\0",sum);
    }
    setHeader(text, true, COLOR_PAIR_DEAFULT);
    drawDiceBoard(COLOR_PAIR_DEAFULT);
    drawDice(die1,die2,COLOR_PAIR_DEAFULT);
    refresh();
    usleep(1000000);
    if(die1 == die2){
        movePlayer(player, moving);
        highlightPlayer(player);
    }
    refresh();
    usleep(300000);
}

// 16. 기금 수령
void get_welfare(int money, int getter, int getter_bud){
    reception(money);
    players[getter].budget = getter_bud;
    drawPlayerInfo(getter);
}

void get_welfare_2(int player, int money, int getter, int getter_bud){
    otherPlayerMessage(player,L"다른 플레이어가 기금을 수령했습니다.",L"접수처");
    players[getter].budget = getter_bud;
    drawPlayerInfo(getter);
}

// 17. 기부
void pay_welfare(int money, int payer, int payer_bud){
    welfare(money);
    players[payer].budget = payer_bud;
    drawPlayerInfo(payer);
}

void pay_welfare_2(int player, int money, int payer, int payer_bud){
    otherPlayerMessage(player,L"다른 플레이어가 기부하였습니다.",L"기부");
    players[payer].budget = payer_bud;
    drawPlayerInfo(payer);

}

// 19. 우대권 여부
int ask_blackcard(int player){
    return blackcard();
}

void ask_blackcard_1(int player){
    otherPlayerMessage(player,L"다른 플레이어가 우대권을 쓸 지 결정중입니다.","우대권");
}

// 20. 우대권 사용 결과
int blackcard_result(){
    setHeader(L"우대권",true,COLOR_PAIR_DEAFULT);
    simpleMessage(L"우대권을 사용하였습니다.","확인");
}

void blackcard_result_2(int player){
    otherPlayerMessage(player,L"우대권을 사용하였습니다.","우대권");
}

// 21. 토지 구매 결과
void buliding_buy_result(int index, int playerID, int building, int playerBud, int buildingToll){
    if(buildingToll == 0)
        return;
    owner[index] = playerID;
    buildings[index] = building;
    cost[index] = buildingToll;
    players[playerID].budget = playerBud;
    drawPlayerInfo(playerID);
    drawTile(index);
}

void otherPlayerMessage(int player, wchar_t* msg, wchar_t* headerMsg){
    highlightPlayer(player);
    clearContext();
    showText(msg);
    setHeader(msg,true,COLOR_PAIR_DEAFULT);
}

void startGame(int player, int pcount){
    currentPlayer = player;
    player_count = pcount;
    setlocale(LC_CTYPE,"");
    if((mainWindow = initscr()) == NULL){
        fprintf(stderr, "Can't init ncurses.\n");
        exit(1);
    }
    maxindex= w * 2 + h * 2 - 4;
    initiate();
    insertTestData();
    clear();
    for(int i = 0;i<maxindex;i++){
        drawTile(i);
    }

    for(int i = 0;i<player_count;i++){
    drawPlayerInfo(i);
    }

    refresh();
}

void endGame(){
    getchar();
    endwin();
}