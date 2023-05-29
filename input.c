#include "game_clnt.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"

#define LEFT 'a'
#define RIGHT 'd'
#define UP 'w'
#define DOWN 's'
#define SPACEBAR ' '

static bool enabled[5];
static bool checked[4];
static int currentCursor = 5;
static int blockPos;




int travelMenuMoveCursor(char dir){
    highlightTile(currentCursor,false);
    switch(dir){
        case UP:{
            if(currentCursor>=w-1 && currentCursor<w+h-2){
                currentCursor++;
            }
            else if(currentCursor>w+w+h-3){
                currentCursor--;
            }
            else if(currentCursor == 0)
                currentCursor = w+w+h+h-5;
            break;
        }
        case DOWN:{
            if(currentCursor>w-1 && currentCursor<=w+h-2){
                currentCursor--;
            }
            else if(currentCursor>=w+w+h-3){
                currentCursor++;
                if(currentCursor == w+w+h+h-4)
                    currentCursor = 0;
            }
            break;
        }
        case LEFT:{
            if(currentCursor<w-1){
                currentCursor++;
            }
            else if(currentCursor>w+h-2 && currentCursor<=w+w+h-3){
                currentCursor--;
            }
            break;
        }
        case RIGHT:{
            if(currentCursor<=w-1){
                currentCursor--;
            }
            else if(currentCursor>=w+h-2 && currentCursor<w+w+h-3){
                currentCursor++;
            }
            break;
        }
        case SPACEBAR:{
            if(currentCursor != blockPos)
                return 1;
        }
    }
    highlightTile(currentCursor,true);
    return 0;
}

int saleMenuMoveCursor(char dir){
    int i;
    if(dir == UP){
        if(currentCursor==5)
            currentCursor = 4;
        for(i=currentCursor-1;i>=1;i--){
            if(enabled[i]){
                currentCursor = i;
                break;
            }
        }
        if(i==0){
            currentCursor = 5;
        }
    }
    else if(dir == DOWN){
        if(currentCursor>=4){
            currentCursor = 0;
        }
        for(i=currentCursor+1;i<=4;i++){
            if(enabled[i]){
                currentCursor = i;
                break;
            }
        }
        if(i == 5)
            currentCursor = i;
    }
    else if(dir == SPACEBAR){
        if(currentCursor<4){
            checked[currentCursor] = !checked[currentCursor];
            bool nochecked = true;
            for(int i = 0;i<4;i++){
                if(checked[i] == true && enabled[i]==true)
                    nochecked = false;
            }
            if(nochecked)
                enabled[4] = false;
            else
                enabled[4] = true;
        }
        else if(currentCursor == 5)
            return 1;
        else
            return 2;
    }
    else if((dir == LEFT || dir == RIGHT) && currentCursor>=4){
        if(currentCursor == 5 && enabled[4]){
            currentCursor = 4;
        }
        else
            currentCursor = 5;
    }
    return 0;
}

int loopTravelMenu(int pos){
    blockPos = pos;
    currentCursor = pos;
    int cmd = 0;
    while(cmd==0){
        updateTravelMenu(currentCursor);
        cmd = travelMenuMoveCursor(getchar());
    }
    setHeader(L"",false,COLOR_PAIR_DEAFULT);
    return currentCursor;
}

int LoopLandSaleMenu(int index, int buildinglist){
    int result = 0;
    InitLandSaleMenu(index, buildinglist);
    int cmd = 0;
    while(cmd==0){
        cmd = saleMenuMoveCursor(getchar());
        updateSaleMenu(index);
    }
    clearContext();
    setHeader(L"",false,COLOR_PAIR_DEAFULT);
    if(currentCursor == 5)
        return 0;
    else{
        int t = 1;
        for(int i = 0;i<4;i++){
            if(checked[i])
                result += t;
            t = t<<1;
        }
    }
    return result;
}

bool yesNoMoveCursor(char dir){
    if(dir==LEFT || dir==RIGHT){
        currentCursor = !currentCursor;
        return false;
    }
    if(dir==SPACEBAR)
        return true;
    return false;
}


bool oneButtonMoveCursor(char dir){
    if(dir==SPACEBAR)
        return true;
    return false;
}

int getYesNo(wchar_t* yesStr, wchar_t* noStr){
    currentCursor = true;
    int cmd = 0;

    while(cmd == 0){
        setTwoButtons(yesStr,noStr,currentCursor==1?COLOR_PAIR_HIGHLIGHT:COLOR_PAIR_DEAFULT, currentCursor==0?COLOR_PAIR_HIGHLIGHT:COLOR_PAIR_DEAFULT);
        refresh();
        cmd = yesNoMoveCursor(getchar());
    }
    return currentCursor;
}


void getOneButton(wchar_t* str){
    currentCursor = true;
    int cmd = 0;
    setOkayButton(str,COLOR_PAIR_HIGHLIGHT);
    refresh();
    while(cmd == 0){
        cmd = oneButtonMoveCursor(getchar());
    }
}

int checkUndertake(int index, int price){
    int t = 1;
    clearContext();
    showTextWithMoney(L"건물을 인수하시겠습니까?",price);
    return getYesNo(L"예",L"아니오");
}

int checkLoan(int index, int price){
    clearContext();
    showText(L"대출권을 사용하시겠습니까?");
    return getYesNo(L"예",L"아니오");    
}

void simpleMessage(wchar_t* msg, wchar_t* okayMessage){
    clearContext();
    showText(msg);
    getOneButton(okayMessage);
}

int checkTravel(int price){
    clearContext();
    showTextWithMoney(L"우주여행을 하시겠습니까?",price);
    return getYesNo(L"예",L"아니오");
}

void updateTravelMenu(int index){
    clearContext();
    setHeader(L"세계여행할 장소를 골라주세요.", true, COLOR_PAIR_DEAFULT);
    setOkayButton(names[currentCursor],COLOR_PAIR_DEAFULT);
    highlightTile(currentCursor,true);
    refresh();
}

void updateSaleMenu(int index){
    setHeader(L"구매할 건물을 선택해주세요.", true, COLOR_PAIR_DEAFULT);
    setTwoButtons(L"확인", L"취소",(!enabled[4]?COLOR_PAIR_FADE:(currentCursor==4?COLOR_PAIR_HIGHLIGHT:COLOR_PAIR_DEAFULT)), (currentCursor==5?COLOR_PAIR_HIGHLIGHT:COLOR_PAIR_DEAFULT));
    setLandSaleContext(names[index], enabled[0], enabled[1], enabled[2], enabled[3], buildingPrice[index], buildingToll[index],
    checked[1],checked[2],checked[3],currentCursor,COLOR_PAIR_DEAFULT, COLOR_PAIR_HIGHLIGHT, COLOR_PAIR_FADE);
    
    refresh();
}
void updateLandsaleMenu(int index, bool sellable){
    clearContext();
    setHeader(L"매각할 땅을 골라주세요. 'Enter'를 눌러 매각합니다.", true, COLOR_PAIR_DEAFULT);
    setOkayButton(L"매각",sellable?COLOR_PAIR_HIGHLIGHT:COLOR_PAIR_DEAFULT);
    refresh();
}

void toll(int visitor, int owner, int price){
    if(owner == -1){
        setTollContext(L"은행원",names[players[visitor].position],price);
    }
    else
        setTollContext(players[owner].name,names[players[visitor].position],price);
    getOneButton(L"확인");
}

void island(int remaining){                                 // 무인도
    wchar_t header[30];
    swprintf(header,30,L"더블이 나오면 탈출! (남은 턴 수 : %d)\0",remaining);
    setHeader(header,true,COLOR_PAIR_DEAFULT);
    drawDiceBoard(COLOR_PAIR_DEAFULT);
    getOneButton(L"주사위 굴리기");
}

void yourTurn(){
    setHeader(L"당신의 턴입니다.",true,COLOR_PAIR_DEAFULT);
    drawDiceBoard(COLOR_PAIR_DEAFULT);
    getOneButton(L"주사위 굴리기");
}

void welfare(int money){                                    // 기부
    setHeader(L"기부",true, COLOR_PAIR_DEAFULT);
    showTextWithMoney(L"사회복지기금을 지불합니다.",money);
    getOneButton(L"지불");
}

void reception(int money){                                  // 접수처
    setHeader(L"접수처",true, COLOR_PAIR_DEAFULT);
    showTextWithMoney(L"접수처에서 복지금을 수령합니다.",money);
    getOneButton(L"수령");
}


int escape(int remaining){
    setHeader(L"무인도",true, COLOR_PAIR_DEAFULT);
    wchar_t header[30];
    swprintf(header,30,L"무인도 탈줄권을 사용하시겠습니까? (남은 턴 수 : %d)\0",remaining);
    showText(header);
    return getYesNo(L"예",L"아니오");    
}

int blackcard(){
    clearContext();
    showText(L"우대권을 사용하시겠습니까?");
    return getYesNo(L"예",L"아니오");

}

void payTax(int fee){
    setHeader(L"황금카드",true, COLOR_PAIR_HIGHLIGHT);
    showTextWithMoney(L"정기종합소득세를 지불하세요!",fee);
}

void getPrize(int money){
    setHeader(L"황금카드",true,COLOR_PAIR_HIGHLIGHT);
    showTextWithMoney(L"노벨상을 수상했습니다!",money);
    
}

void InitLandSaleMenu(int index, int buildinglist){
    enabled[4] = true;
    for(int i = 1;i<4;i++)
        checked[i] = false;
    currentCursor = 5;


    int t = buildinglist;
    for(int i = 0;i<4;i++){
        enabled[i] = !(t&1);
        checked[i] = !enabled[i];
        t = t>>1;
    }
    checked[0] = true;
    for(int i = 1;i<4;i++){
        if(enabled[i]){
            currentCursor = i;
            break;
        }
    }
    if(enabled[0]==false && currentCursor==5){
        enabled[4] = false;
    }

    updateSaleMenu(index);
}


int landsaleMenuMoveCursor(char dir, int landown, int landlist[4], bool checked_tile[28], bool sellable){
    if(!checked_tile[currentCursor])
        highlightTile(currentCursor,false);
    switch(dir){
        case UP:{
            if(currentCursor>=w-1 && currentCursor<w+h-2){
                currentCursor++;
            }
            else if(currentCursor>w+w+h-3){
                currentCursor--;
            }
            else if(currentCursor == 0)
                currentCursor = w+w+h+h-5;
            break;
        }
        case DOWN:{
            if(currentCursor>w-1 && currentCursor<=w+h-2){
                currentCursor--;
            }
            else if(currentCursor>=w+w+h-3){
                currentCursor++;
                if(currentCursor == w+w+h+h-4)
                    currentCursor = 0;
            }
            break;
        }
        case LEFT:{
            if(currentCursor<w-1){
                currentCursor++;
            }
            else if(currentCursor>w+h-2 && currentCursor<=w+w+h-3){
                currentCursor--;
            }
            break;
        }
        case RIGHT:{
            if(currentCursor<=w-1){
                currentCursor--;
            }
            else if(currentCursor>=w+h-2 && currentCursor<w+w+h-3){
                currentCursor++;
            }
            break;
        }
        case SPACEBAR:{
            if(hasLand(currentCursor,landown)){
                if(checked_tile[currentCursor]){
                    highlightTile(currentCursor,true);
                    return 2;
                }
                else{
                    highlightTileBlue(currentCursor,true);
                    return 3;
                }
            }
            break;
        }

        case 'v':{
            return 1;
        }
    }
    if(checked_tile[currentCursor]){        
        highlightTileBlue(currentCursor,true);
    }
    else
        highlightTile(currentCursor,true);
    return 0;
}

int getLandPrice(int index, const int landList[4]){
    int arri = index/8;
    int elemi = (index%8) * 4;
    int bits = landList[arri]>>elemi;
    int total = 0;
    for(int i = 0;i<4;i++){
        if(bits&1){
            total += buildingPrice[index][i]/2;
        }
        bits>>=1;
    }
    return total;
}

int hasLand(int index, int binary){
    return (binary&1<<index);
}

int loolLandsaleMenu(int pos, int needPrice, int landown, int list1, int list2, int list3, int list4){
    bool checked_tile[28] = {false,};
    int landOwn = landown;                    //binary
    int landlist[4] = {list1,list2,list3,list4};                //4bit each
    bool sellable = false;
    int total = 0;

    blockPos = pos;
    currentCursor = pos;
    int cmd = 0;
    while(true){
        updateLandsaleMenu(currentCursor,sellable);
        setSellLandContext(needPrice, total);
        cmd = landsaleMenuMoveCursor(getchar(),landOwn,landlist,checked_tile,sellable);
        if(cmd == 2){
            checked_tile[currentCursor] = false;
            total -= getLandPrice(currentCursor,landlist);
            highlightTileBlue(currentCursor,false);
            highlightTile(currentCursor,true);
        }
        else if(cmd == 3){            
            checked_tile[currentCursor] = true;
            total += getLandPrice(currentCursor,landlist);
            highlightTileBlue(currentCursor,true);
        }

        if(needPrice<= total){
            sellable = true;
        }
        else{
            sellable = false;
        }
        if(cmd == 1){
            if(sellable)
                break;
        }
    }
    int result = 0;
    int t = 1;
    for(int i = 0;i<28;i++){
        if(checked_tile[i]){
            result += t;
        }
        t<<=1;
    }
    setHeader(L"",false,COLOR_PAIR_DEAFULT);

    for(int i = 0;i<28;i++){
        highlightTileBlue(i,false);
    }
    return result;
}