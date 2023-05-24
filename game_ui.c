#include "game_clnt.h"

char* blockStr;
wchar_t* blockUpperHalfStr;
wchar_t* blockLowerHalfStr;

char* headerStr;
char* headerErase;

int blockw = 12;
int blockh = 6;

int paddingx = 2;
int paddingy = 1;

int panel_width;
int panel_x;
int panel_height;
int panel_y;

int currentTurn = 0;

int getX(int);
int getY(int);

int maxindex;
Player players[4];
int player_count = 4;

int cost[28];
int owner[28]; 
int buildings[28];
int buildingPrice[28][4];
int buildingToll[28][4];

wchar_t* names[28] = {
    L"출발", L"타이베이", L"황금 열쇠", L"베이징", L"마닐라", L"제주도", L"싱가포르",
    L"무인도", L"아테네", L"황금 열쇠", L"코펜하겐", L"스페이스X", L"베를린", L"오타와",
    L"복지금", L"상파울루", L"황금 열쇠", L"부산", L"하와이", L"타이타닉", L"마드리드",
    L"우주여행", L"도쿄", L"컬럼비아호", L"런던", L"황금 열쇠", L"기부",L"서울"
};

int types[] = {
    T_START, T_N, T_KEY, T_N, T_N, T_N, T_N,
    T_ISLAND, T_N, T_KEY, T_N, T_N, T_N, T_N,
    T_WELFARE, T_N, T_KEY, T_N, T_N, T_N, T_N, 
    T_SPACE, T_N, T_N, T_N, T_KEY, T_WELFARE, T_N};
WINDOW* mainWindow;

int calcCost(int index){
    int t= 1;
    int _cost = 0;
    for(int i = 0;i<4;i++){
        if(buildings[index]&t){
            _cost += buildingToll[index][i];
        }
        t= t<<1;
    }
    cost[index] = _cost;
    return _cost;
}

void setOkayButton(wchar_t* text, int color){
    int width = 20;
    int len = wcslen(text)*2;
    int offset = (width-len-2)/2+1;
    wattron(mainWindow,COLOR_PAIR(color));
    move(panel_y + panel_height - 8,panel_x + (panel_width - width)/2);
    addwstr(L"┏━━━━━━━━━━━━━━━━━━┓");                  // width : 20
    move(panel_y + panel_height - 7,panel_x + (panel_width - width)/2);
    addwstr(L"┃                  ┃");                  // width : 20
    move(panel_y + panel_height - 7 ,panel_x + (panel_width - width)/2 + offset);
    addwstr(text);                  // width : 20
    move(panel_y + panel_height - 6,panel_x + (panel_width - width)/2);
    addwstr(L"┗━━━━━━━━━━━━━━━━━━┛");
}

void setTwoButtons(wchar_t* text1, wchar_t* text2, int color1, int color2){
    int buttonWidth = 15;
    int buttonMargin = 10;
    int len1 = wcslen(text1)*2;
    int len2 = wcslen(text2)*2;
    int offset1 = (buttonWidth-2-len1)/2+1;
    int offset2 = (buttonWidth-2-len2)/2+1;
    wattron(mainWindow,COLOR_PAIR(color1));
    move(panel_y + panel_height - 8,panel_x + (panel_width - buttonWidth *2 - buttonMargin)/2);
    addwstr(L"┏━━━━━━━━━━━━━┓");                  // width : 15
    move(panel_y + panel_height - 7,panel_x + (panel_width - buttonWidth *2 - buttonMargin)/2);
    addwstr(L"┃             ┃");                  // width : 20
    move(panel_y + panel_height - 7 ,panel_x + (panel_width - buttonWidth *2 - buttonMargin)/2 + offset1);
    addwstr(text1);                  // width : 20
    move(panel_y + panel_height - 6,panel_x + (panel_width - buttonWidth *2 - buttonMargin)/2);
    addwstr(L"┗━━━━━━━━━━━━━┛");
    
    wattron(mainWindow,COLOR_PAIR(color2));
    move(panel_y + panel_height - 8,panel_x + (panel_width - buttonWidth *2 - buttonMargin)/2 + (buttonWidth + buttonMargin));
    addwstr(L"┏━━━━━━━━━━━━━┓");                  // width : 15
    move(panel_y + panel_height - 7,panel_x + (panel_width - buttonWidth *2 - buttonMargin)/2 + (buttonWidth + buttonMargin));
    addwstr(L"┃             ┃");                  // width : 20
    move(panel_y + panel_height - 7 ,panel_x + (panel_width - buttonWidth *2 - buttonMargin)/2 + (buttonWidth + buttonMargin) + offset2);
    addwstr(text2);                  // width : 20
    move(panel_y + panel_height - 6,panel_x + (panel_width - buttonWidth *2 - buttonMargin)/2 + (buttonWidth + buttonMargin));
    addwstr(L"┗━━━━━━━━━━━━━┛");
}

void clearContext(){
    wattron(mainWindow,COLOR_PAIR(COLOR_PAIR_DEAFULT));    
    for(int i = panel_y + 8;i<panel_y + panel_height-5;i++){
        move(i,panel_x);
        addstr(headerErase);
    }
}

void drawPlayerInfo(int player){
    int x,y;
    switch(player){
        case 0:
            x = panel_x+1;
            y = panel_y;
            break;
        case 1:
            x = panel_x + panel_width - 26;
            y = panel_y;
            break;
        case 2:
            x = panel_x+1;
            y = panel_y + panel_height - 4;
            break;
        case 3:
            x = panel_x + panel_width - 26;
            y = panel_y + panel_height - 4;
            break;
    }

    if(player == currentTurn){
        wattron(mainWindow,COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    }
    else{
        wattron(mainWindow,COLOR_PAIR(COLOR_PAIR_DEAFULT));
    }
    move(y,x);
    addwstr(L"┏━━━━━━━━━━━━━━━━━━━━━━━┓");                  // width : 25
    char s1[31];
    char s2[31];
    sprintf(s1,"┃P%d: %-19s┃",player+1,players[player].name);
    sprintf(s2,"┃￦%-21d┃",players[player].budget);
    move(y+1,x);
    addstr(s1);
    move(y+2,x);
    addstr(s2);
    move(y+3,x);
    addwstr(L"┗━━━━━━━━━━━━━━━━━━━━━━━┛");
}

int getColorPair(int player, int cPair){
    return cPair + 16 * (player+1);
}

int getPlayerColorPair(int player, bool alt){
    int result = player * 2 + 8;
    if(alt) result += 1;
    return result;
}

int getHighlightColorPair(int backColor){
    return COLOR_PAIR_NUM * 5 + backColor;
}


void drawBuilding(int building, int x, int y, int player, int color){
    y+= 1;
    x+= 4;
    building = building>>1;
    wattron(mainWindow, COLOR_PAIR(color));
    
    wchar_t* buildingIcons[] = {L"□",L"▥",L"▒"};
    for(int i = 0;i<3;i++){
        move(y,x);
        if(building & 1){
            addwstr(buildingIcons[i]);
        }
        else{
            addwstr(L"_");
        }
        x++;        
        building = building>>1;
    }
}

void setHeader(wchar_t* text, bool show, int color){
    wattron(mainWindow, COLOR_PAIR(color));
    if(!show){
        move(panel_y + 5, panel_x);
        addstr(headerErase);
        move(panel_y + 6, panel_x);
        addstr(headerErase);
        move(panel_y + 7, panel_x);
        addstr(headerErase);
    }
    else{
        move(panel_y + 5, panel_x);
        addstr(headerStr);
        move(panel_y + 7, panel_x);
        addstr(headerStr);

        move(panel_y + 6, panel_x);
        addstr(headerErase);
        int len = wcslen(text)*2;
        int offset = (panel_width-2-len)/2;
        move(panel_y + 6, panel_x + offset);
        addwstr(text);
    }
}

void drawPrice(int price, int x, int y, int color){
    y+=3;
    wattron(mainWindow, COLOR_PAIR(color));
    char str[16];
    sprintf(str,"￦%d",price);
    int len = strlen(str);    
    int offset = (blockw - len)/2;
    move(y,x+offset);
    addstr(str);
}

void drawMarker(bool p1, bool p2, bool p3, bool p4, int x, int y, int color){
    y+=blockh-2;
    x+= 2;
    if(p1){
        move(y,x);
        wattron(mainWindow, COLOR_PAIR(getColorPair(0,color)));
        addwstr(L"●");
    }
    if(p2){
        move(y,x+2);
        wattron(mainWindow, COLOR_PAIR(getColorPair(1,color)));
        addwstr(L"●");
    }
    if(p3){
        move(y,x+4);
        wattron(mainWindow, COLOR_PAIR(getColorPair(2,color)));
        addwstr(L"●");
    }
    if(p4){
        move(y,x+6);
        wattron(mainWindow, COLOR_PAIR(getColorPair(3,color)));
        addwstr(L"●");
    }
}

void drawbox(int x, int y, int color){
    for(int i = y;i<y+blockh-1;i++){
        move(i,x);
        wattron(mainWindow, COLOR_PAIR(color));
        addstr(blockStr);
    }
        move(y+blockh-1,x);
        wattron(mainWindow, COLOR_PAIR(color));
        addstr(blockStr);
}

void drawLabel(wchar_t* text, int x, int y, int color){
    y+=2;
    int len = wcslen(text)*2;
    
    int offset = (blockw - len)/2;
    move(y,x+offset);
    wattron(mainWindow, COLOR_PAIR(color));
    addwstr(text);    
}

int getBgColorByIndex(int index){

    int t = 0;    
    switch(types[index]){
        case T_START:
            t = COLOR_PAIR_GREEN;
            break;
        case T_N:
            if(owner[index] != -1){
                t= getPlayerColorPair(owner[index],(index%2));
            }
            else{
                t = (index%2)?COLOR_PAIR_BLOCK_GRAY:COLOR_PAIR_BLOCK_GRAY_LIGHT;
            }
            break;
        case T_KEY:
            t = COLOR_PAIR_BLOCK_YELLOW;         
            break;  
        case T_ISLAND:
            t = COLOR_PAIR_BLUE;
            break;
        case T_SPACE:
            t= COLOR_PAIR_BLUE;
            break;
        case T_WELFARE:
            t = COLOR_PAIR_BLUE;
            break;
    }
    return t;
}

void drawTile(int index){
    if(index>=maxindex){
        fprintf(stderr,"index out of range!\n");
        return;
    }
    bool pVisit[4];
    memset(pVisit,false,sizeof(bool)*4);

    for(int i = 0;i<player_count;i++){
        if(players[i].position == index)
            pVisit[i] = true;
    }

    int bgColor = getBgColorByIndex(index);

    drawbox(getX(index),getY(index),bgColor);
    drawLabel(names[index],getX(index),getY(index),bgColor);
    drawMarker(pVisit[0],pVisit[1],pVisit[2],pVisit[3],getX(index),getY(index),bgColor);

    if(types[index]== T_N && owner[index]!=-1){
                calcCost(index);
                drawPrice(cost[index],getX(index),getY(index),bgColor);
                drawBuilding(buildings[index],getX(index),getY(index),owner[index],bgColor);
    }

    refresh();
}

void highlightTile(int index, bool highlight){
    int color;
    int x,y;
    x = getX(index);
    y = getY(index);
    if(highlight)
        color = getHighlightColorPair(getBgColorByIndex(index));
    else
        color = getBgColorByIndex(index);
    wattron(mainWindow, COLOR_PAIR(color));
    int i;
    if(highlight){
        move(y,x+1);
        addwstr(blockUpperHalfStr);
    }
    else{
        move(y,x);
        addstr(blockStr);
    }
    for(i = 0;i<blockh;i++){
        if(highlight){
            move(y+i,x);
            addwstr(L"█");
            move(y+i,x+blockw-1);
            addwstr(L"█");
        }
        else{
            move(y+i,x);
            addstr(" ");
            move(y+i,x+blockw-1);
            addstr(" ");
        }
    }
    if(highlight){
        move(y+i-1,x+1);
        addwstr(blockLowerHalfStr);
    }
    else{
        move(y+i-1,x);
        addstr(blockStr);
    }
}
typedef struct Square {
    int type;       // 0: 도시, 1: 특수도시, 2: 황금열쇠 3: 기타
    int buildings;  // hotel: 1000, building: 0100, villa: 0010, land: 0001
    int owner;
    const int price[4]; // hotel, building, villa, land
    const int toll[4];  // hotel, building, villa, land
} Square;

void insertTestData(){    
    const int MAN = 10;
    Square squares[28] = {
        {3, 0, -1, {0,}, {0,}},                                                              // 시작
        {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 5*MAN}, {25*MAN, 9*MAN, 1*MAN, 0.2*MAN}},         // 타이페이
        {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
        {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 8*MAN}, {45*MAN, 18*MAN, 2*MAN, 0.4*MAN}},        // 베이징
        {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 8*MAN}, {45*MAN, 18*MAN, 2*MAN, 0.4*MAN}},        // 마닐라
        {1, 0, -1, {0,0,0,20*MAN}, {0,0,0,30*MAN}},                                          // 제주도
        {0, 0, -1, {25*MAN, 15*MAN, 5*MAN, 10*MAN}, {55*MAN, 27*MAN, 3*MAN, 0.6*MAN}},       // 싱가포르
        {3, 0, -1, {0,}, {0,}},                                                              // 무인도
        {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 14*MAN}, {75*MAN, 45*MAN, 5*MAN, 1*MAN}},        // 아테네
        {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
        {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 16*MAN}, {75*MAN, 45*MAN, 6*MAN, 1*MAN}},        // 코펜하겐
        {1, 0, -1, {0,0,0,20*MAN}, {0,0,0,30*MAN}},                                          // 콩코드 여객기
        {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 18*MAN}, {95*MAN, 55*MAN, 7*MAN, 1.4*MAN}},      // 베를린
        {0, 0, -1, {50*MAN, 30*MAN, 10*MAN, 20*MAN}, {100*MAN, 60*MAN, 8*MAN, 1.6*MAN}},     // 몬트리올
        {3, 0, -1, {0,}, {0,0,0,0}},                                                         // 사회복지기금 
        {0, 0, -1, {75*MAN, 45*MAN, 15*MAN, 24*MAN}, {110*MAN, 75*MAN, 10*MAN, 2*MAN}},      // 상파울루
        {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
        {1, 0, -1, {0,0,0,50*MAN}, {0,0,0,60*MAN}},                                          // 부산
        {0, 0, -1, {75*MAN, 45*MAN, 15*MAN, 26*MAN}, {115*MAN, 80*MAN, 11*MAN, 2.2*MAN}},    // 하와이
        {1, 0, -1, {0,0,0,30*MAN}, {0,0,0,25*MAN}},                                          // 퀸 엘리자베스호
        {0, 0, -1, {75*MAN, 45*MAN, 15*MAN, 26*MAN}, {115*MAN, 80*MAN, 11*MAN, 2.2*MAN}},    // 마드리드
        {3, 0, -1, {0,}, {0,0,0,20*MAN}},                                                    // 우주여행
        {0, 0, -1, {100*MAN, 60*MAN, 20*MAN, 30*MAN}, {127*MAN, 90*MAN, 13*MAN, 2.6*MAN}},   // 도쿄
        {1, 0, -1, {0,0,0,45*MAN}, {0,0,0,30*MAN}},                                          // 컬럼비아호
        {0, 0, -1, {100*MAN, 60*MAN, 20*MAN, 35*MAN}, {150*MAN, 110*MAN, 17*MAN, 3.5*MAN}},  // 런던
        {2, 0, -1, {0,}, {0,}},                                                              // 황금열쇠
        {3, 0, -1, {0,}, {0,0,0,15*MAN}},                                                    // 기부
        {1, 0, -1, {0,0,0,100*MAN}, {0,0,0,200*MAN}}                                         // 서울
    };

    for(int i = 0;i<28;i++){
        for(int j = 0;j<4;j++){
            buildingPrice[i][j] = squares[i].price[3-j];
            buildingToll[i][j] = squares[i].toll[3-j];
        }
    }
}

void initiate(){
    noecho();
    curs_set(0);
    for(int i= 0;i<maxindex;i++){
        owner[i] = -1;
    }
    panel_width = blockw * (w-2) -2;
    panel_height = blockh * (h-2) -2;
    panel_x = paddingx + blockw + 1;
    panel_y = paddingy + blockh + 1;

    blockStr = malloc(sizeof(char) * (blockw + 1));
    for(int i = 0;i<blockw;i++){
        blockStr[i] = ' ';
    }
    blockStr[blockw] = '\0';

    blockLowerHalfStr = malloc(sizeof(wchar_t) * (blockw -1));
    blockUpperHalfStr = malloc(sizeof(wchar_t) * (blockw -1));

    for(int i = 0;i<blockw-2;i++){
        blockLowerHalfStr[i] = L'▄';
        blockUpperHalfStr[i] = L'▀';
    }
    blockLowerHalfStr[blockw-2] = '\0';
    blockUpperHalfStr[blockw-2] = '\0';

    headerStr = malloc(sizeof(char) * (panel_width+1));
    headerErase = malloc(sizeof(char) * (panel_width+1));

    for(int i = 1;i<panel_width-1;i++){
        headerStr[i] = '=';
        headerErase[i] = ' ';
    }
    headerStr[0] = ' ';
    headerErase[0] = ' ';
    headerStr[panel_width] = '\0';
    headerErase[panel_width] = '\0';

    start_color();
    init_pair(COLOR_PAIR_DEAFULT,COLOR_WHITE,COLOR_BLACK);
    init_pair(COLOR_PAIR_HIGHLIGHT,229,COLOR_BLACK);
    init_pair(COLOR_PAIR_BLOCK_YELLOW, 0, BLOCK_YELLOW_BACK);
    init_pair(COLOR_PAIR_BLOCK_YELLOW_LIGHT, 0, BLOCK_YELLOW_LIGHT_BACK);
    init_pair(COLOR_PAIR_BLOCK_GRAY,0, BLOCK_GRAY_BACK);
    init_pair(COLOR_PAIR_BLOCK_GRAY_LIGHT,0, BLOCK_GRAY_LIGHT_BACK);
    init_pair(COLOR_PAIR_BLUE,0, BLOCK_BLUE_BACK);
    init_pair(COLOR_PAIR_GREEN, 0, BLOCK_GREEN_BACK);
    init_pair(COLOR_PAIR_P1,0,BLOCK_P1_BACK);
    init_pair(COLOR_PAIR_P1_ALT,0,BLOCK_P1_ALT_BACK);
    init_pair(COLOR_PAIR_P2,0,BLOCK_P2_BACK);
    init_pair(COLOR_PAIR_P2_ALT,0,BLOCK_P2_ALT_BACK);
    init_pair(COLOR_PAIR_P3,0,BLOCK_P3_BACK);
    init_pair(COLOR_PAIR_P3_ALT,0,BLOCK_P3_ALT_BACK);
    init_pair(COLOR_PAIR_P4,0,BLOCK_P4_BACK);
    init_pair(COLOR_PAIR_P4_ALT,0,BLOCK_P4_ALT_BACK);
    init_pair(COLOR_PAIR_WHITE,15,15);
    init_pair(COLOR_PAIR_P1_DARK,COLOR_WHITE,P1_MARKER);
    init_pair(COLOR_PAIR_P2_DARK,COLOR_WHITE,P2_MARKER);
    init_pair(COLOR_PAIR_P3_DARK,COLOR_WHITE,P3_MARKER);
    init_pair(COLOR_PAIR_P4_DARK,COLOR_WHITE,P4_MARKER);
    init_pair(COLOR_PAIR_GRAY_DARK,COLOR_WHITE,BLOCK_DARK_GRAY);
    init_pair(COLOR_PAIR_FADE,BLOCK_DARK_GRAY,COLOR_BLACK);

    int pColors[] = {0,P1_MARKER,P2_MARKER,P3_MARKER,P4_MARKER};

    for(int i = 1;i<=4;i++){
        init_pair(COLOR_PAIR_NUM * i + 1,pColors[i],COLOR_BLACK);
        init_pair(COLOR_PAIR_NUM * i + 2,pColors[i],BLOCK_YELLOW_BACK);
        init_pair(COLOR_PAIR_NUM * i + 3,pColors[i],BLOCK_YELLOW_LIGHT_BACK);
        init_pair(COLOR_PAIR_NUM * i + 4,pColors[i],BLOCK_GRAY_BACK);
        init_pair(COLOR_PAIR_NUM * i + 5,pColors[i],BLOCK_GRAY_LIGHT_BACK);
        init_pair(COLOR_PAIR_NUM * i + 6,pColors[i],BLOCK_BLUE_BACK);
        init_pair(COLOR_PAIR_NUM * i + 7,pColors[i],BLOCK_GREEN_BACK);
        init_pair(COLOR_PAIR_NUM * i + 8,pColors[i],BLOCK_P1_BACK);
        init_pair(COLOR_PAIR_NUM * i + 9,pColors[i],BLOCK_P1_ALT_BACK);
        init_pair(COLOR_PAIR_NUM * i + 10,pColors[i],BLOCK_P2_BACK);
        init_pair(COLOR_PAIR_NUM * i + 11,pColors[i],BLOCK_P2_ALT_BACK);
        init_pair(COLOR_PAIR_NUM * i + 12,pColors[i],BLOCK_P3_BACK);
        init_pair(COLOR_PAIR_NUM * i + 13,pColors[i],BLOCK_P3_ALT_BACK);
        init_pair(COLOR_PAIR_NUM * i + 14,pColors[i],BLOCK_P4_BACK);
        init_pair(COLOR_PAIR_NUM * i + 15,pColors[i],BLOCK_P4_ALT_BACK);
    }
    
    init_pair(COLOR_PAIR_NUM * 5 + 1,COLOR_RED,COLOR_BLACK);
    init_pair(COLOR_PAIR_NUM * 5 + 2,COLOR_RED,BLOCK_YELLOW_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 3,COLOR_RED,BLOCK_YELLOW_LIGHT_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 4,COLOR_RED,BLOCK_GRAY_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 5,COLOR_RED,BLOCK_GRAY_LIGHT_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 6,COLOR_RED,BLOCK_BLUE_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 7,COLOR_RED,BLOCK_GREEN_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 8,COLOR_RED,BLOCK_P1_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 9,COLOR_RED,BLOCK_P1_ALT_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 10,COLOR_RED,BLOCK_P2_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 11,COLOR_RED,BLOCK_P2_ALT_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 12,COLOR_RED,BLOCK_P3_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 13,COLOR_RED,BLOCK_P3_ALT_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 14,COLOR_RED,BLOCK_P4_BACK);
    init_pair(COLOR_PAIR_NUM * 5 + 15,COLOR_RED,BLOCK_P4_ALT_BACK);
}

int getX(int blockindex){
    int t;
    if(blockindex<w){
        t = (w-blockindex-1);
    }
    else if(blockindex<w+h-1){
        t = 0;
    }
    else if(blockindex<w+w+h-2){
        t = blockindex + 2 - h - w;
    }
    else{
        t = w-1;
    }
    return (t * blockw) + paddingx;
}

int getY(int blockindex){
    int t;
    if(blockindex<w){
        t = h-1;
    }
    else if(blockindex<w+h-1){
        t = h - (blockindex -w + 1) - 1;
    }
    else if(blockindex<w+w+h-2){
        t = 0;
    }
    else{
        t = blockindex - w - w - h + 3;
    }
    return (t * blockh) + paddingy;
}

void showTextWithMoney(wchar_t* str, int money){    
    wattron(mainWindow,COLOR_PAIR(COLOR_PAIR_DEAFULT));    
    wchar_t str1[49];
    int width = 50;
    int left = panel_x + (panel_width - width)/2;
    swprintf(str1,49,L"비용 : ￦%d\0",money);
    int offset0 = (width-wcslen(str)*2)/2+1;
    int offset1 = (width-wcslen(str1)*2)/2+1;

    move(panel_y + 9, left);
    addwstr(L"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");                  // width : 20
    move(panel_y + 10, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 11, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 11, left + offset0);
    addwstr(str);
    move(panel_y + 12, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 12, left + offset1);
    addwstr(str1);
    move(panel_y + 13, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 14, left);
    addwstr(L"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
}

void showText(wchar_t* str){
    wattron(mainWindow,COLOR_PAIR(COLOR_PAIR_DEAFULT));    
    int width = 50;
    int left = panel_x + (panel_width - width)/2;
    int offset0 = (width-wcslen(str)*2)/2+1;

    move(panel_y + 9, left);
    addwstr(L"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");                  // width : 20
    move(panel_y + 10, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 11, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 11, left + offset0);
    addwstr(str);
    move(panel_y + 12, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 13, left);
    addwstr(L"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
}

void drawDiceBoard(int color){
    int width = 30;
    int left = panel_x + (panel_width - width)/2;
    wattron(mainWindow, COLOR_PAIR(color));
    move(panel_y + 10, left);
    addwstr(L"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
    move(panel_y + 11, left);
    addwstr(L"┃                            ┃");
    move(panel_y + 12, left);
    addwstr(L"┃                            ┃");
    move(panel_y + 13, left);
    addwstr(L"┃                            ┃");
    move(panel_y + 14, left);
    addwstr(L"┃                            ┃");
    move(panel_y + 15, left);
    addwstr(L"┃                            ┃");
    move(panel_y + 16, left);
    addwstr(L"┃                            ┃");
    move(panel_y + 17, left);
    addwstr(L"┃                            ┃");
    move(panel_y + 18, left);
    addwstr(L"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
}

void drawDice(int die1, int die2,int color){
    die1--;
    die2--;
    int y = panel_y + 12;
    int left = panel_x + panel_width/2 -10;
    static wchar_t* dice[6][5] = {
        {
        L"┏━━━━━━━┓",
        L"┃       ┃",
        L"┃   ●   ┃",
        L"┃       ┃",
        L"┗━━━━━━━┛"},
        {
        L"┏━━━━━━━┓",
        L"┃     ● ┃",
        L"┃       ┃",
        L"┃ ●     ┃",
        L"┗━━━━━━━┛"},
        {
        L"┏━━━━━━━┓",
        L"┃ ●     ┃",
        L"┃   ●   ┃",
        L"┃     ● ┃",
        L"┗━━━━━━━┛"},
        {
        L"┏━━━━━━━┓",
        L"┃ ●   ● ┃",
        L"┃       ┃",
        L"┃ ●   ● ┃",
        L"┗━━━━━━━┛"},
        {
        L"┏━━━━━━━┓",
        L"┃ ●   ● ┃",
        L"┃   ●   ┃",
        L"┃ ●   ● ┃",
        L"┗━━━━━━━┛"},
        {
        L"┏━━━━━━━┓",
        L"┃ ●   ● ┃",
        L"┃ ●   ● ┃",
        L"┃ ●   ● ┃",
        L"┗━━━━━━━┛"}
    };   

    wattron(mainWindow, COLOR_PAIR(color));
    for(int i = 0;i<5;i++){
        move(y+i,left);
        addwstr(dice[die1][i]);
        move(y+i,left+11);
        addwstr(dice[die2][i]);
    }
}

void setTollContext(wchar_t* owner, wchar_t* name, int toll){
    int width = 50;    
    int left = panel_x + (panel_width - width)/2;
    wchar_t* str1 = L"통행세를 지불합니다.";
    wchar_t str2[51];

    swprintf(str2,51,L"지불액 : ￦%d\0",toll);
    int offset0 = (width-wcslen(name)*2)/2+1;
    int offset1 = (width-wcslen(str1)*2)/2+1;
    int offset2 = (width-wcslen(str2)*2)/2+1;
    move(panel_y + 9, left);
    addwstr(L"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");                  // width : 20
    move(panel_y + 10, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 10, left+ offset0);
    addwstr(name);
    move(panel_y + 11, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 11, left+ offset1);
    addwstr(str1);
    move(panel_y + 12, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 12, left+ offset1);
    addwstr(str2);
    move(panel_y + 13, left);
    addwstr(L"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");

}

void setLandSaleContext(wchar_t* name, bool enable_land, bool enable_b1, bool enable_b2, bool enable_b3, 
            int price[4], int toll[4], bool toggle_b1, bool toggle_b2, bool toggle_b3, 
            int selection, int normal_color, int selected_color, int fade_color){
    wattron(mainWindow, COLOR_PAIR(normal_color));
    int width = 50;
    int left = panel_x + (panel_width - width)/2;

    int offset = (width-wcslen(name)*2-2)/2+1;

    int total_price = 0;
    int total_toll = toll[0];


    if(enable_land){
        total_price += price[0];
    }

    if(toggle_b1){
        total_toll += toll[1];
        if(enable_b1)
            total_price += price[1];
    }

    if(toggle_b2){
        total_toll += toll[2];
        if(enable_b2)
            total_price += price[2];
    }

    if(toggle_b3){
        total_toll += toll[3];
        if(enable_b3)
            total_price += price[3];
    }


    move(panel_y + 9, left);
    addwstr(L"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");                  // width : 20
    move(panel_y + 10, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 11, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 11, left + offset);
    addwstr(name);
    move(panel_y + 12, left);
    addwstr(L"┃                                                ┃");
    move(panel_y + 13, left);
    addwstr(L"┠────────┬──────────────────┬────────────────────┨");
    move(panel_y + 14, left);
    addwstr(L"┃        │                                       ┃");
    move(panel_y + 14, left+1);

    if(!enable_land)
        wattron(mainWindow, COLOR_PAIR(fade_color));
    else if(selection == 0)
        wattron(mainWindow, COLOR_PAIR(selected_color));
    printw(" %ls  %6ls│ ￦%-14d │ +￦%-16d",L"■",L"토지",price[0],toll[0]);
    wattron(mainWindow, COLOR_PAIR(normal_color));
    move(panel_y + 15, left);
    addwstr(L"┃        │                  │                    ┃");
    move(panel_y + 16, left);
    addwstr(L"┃        │                  │                    ┃");
    move(panel_y + 16, left+1);
    
    if(!enable_b1)
        wattron(mainWindow, COLOR_PAIR(fade_color));
    else if(selection == 1)
        wattron(mainWindow, COLOR_PAIR(selected_color));
    printw(" %ls  %6ls│ ￦%-14d │ +￦%-16d",(toggle_b1?L"■":L"□"),L"별장",price[1], toll[1]);
    wattron(mainWindow, COLOR_PAIR(normal_color));
    move(panel_y + 17, left);
    addwstr(L"┃        │                  │                    ┃");
    move(panel_y + 18, left);
    addwstr(L"┃        │                  │                    ┃");
    move(panel_y + 18, left+1);
    
    if(!enable_b2)
        wattron(mainWindow, COLOR_PAIR(fade_color));
    else if(selection == 2)
        wattron(mainWindow, COLOR_PAIR(selected_color));
    printw(" %ls  %6ls│ ￦%-14d │ +￦%-16d",(toggle_b2?L"■":L"□"),L"빌딩",price[2], toll[2]);
    wattron(mainWindow, COLOR_PAIR(normal_color));
    move(panel_y + 19, left);
    addwstr(L"┃        │                  │                    ┃");
    move(panel_y + 20, left);
    addwstr(L"┃        │                  │                    ┃");
    move(panel_y + 20, left+1);
    
    if(!enable_b3)
        wattron(mainWindow, COLOR_PAIR(fade_color));
    else if(selection == 3)
        wattron(mainWindow, COLOR_PAIR(selected_color));
    printw(" %ls  %6ls│ ￦%-14d │ +￦%-16d",(toggle_b3?L"■":L"□"),L"호텔",price[3], toll[3]);
    wattron(mainWindow, COLOR_PAIR(normal_color));
    move(panel_y + 21, left);
    addwstr(L"┠────────┼──────────────────┼────────────────────┨");
    move(panel_y + 22, left);
    printw("┃ 지불액 │ ￦%-15d│ 통행세: ￦%-9d┃",total_price,total_toll);
    move(panel_y + 23, left);
    addwstr(L"┗━━━━━━━━┷━━━━━━━━━━━━━━━━━━┷━━━━━━━━━━━━━━━━━━━━┛");
}

void highlightPlayer(int player){
    highlightTile(players[currentTurn].position,false);
    highlightTile(players[player].position,true);
    int t = currentTurn;
    currentTurn = player;
    drawPlayerInfo(t);
    drawPlayerInfo(player);
}

void movePlayer(int movingID, int movingPosition){    
        highlightTile(players[movingID].position,false);
        int t = players[movingID].position;
        players[movingID].position = movingPosition;

        highlightTile(players[movingID].position,true);
        drawTile(movingPosition);
        drawTile(t);
}