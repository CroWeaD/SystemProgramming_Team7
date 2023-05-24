#define MAX_LOBBY 20
#define MAXLEN 300

/* Values for result */
#define FAIL    -10
#define SUCCESS 10
#define QUIT    -100

#define NO 0
#define YES 1

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 8080

typedef struct{
    char id[MAXLEN];
    char password[MAXLEN];
    char username[MAXLEN];
}user_info;

typedef struct{
    int id;
	char title[MAXLEN];
	int accessible;
	int users;
    int port;
}Lobby;


//type int 1개
//주사위 int 2개
//비용 int 1개
//칸 번호 int 1개
//빌딩 int 1개
//토지 소유 여부 int 1개
//남은 턴 int 1개

typedef struct{
    char player[MAXLEN];
    int type;       //
    int dice[2];    //
    int price;      //
    int square;     //
    int building;   //
    int own;        //
    int turn;       //9.
	//int data[10];
    //add
}Game_Send;

typedef struct{
    int player;
    int type;
	int data[2];
    //add
}Game_Recv;

typedef struct PACKET{
    int result;
    char message[MAXLEN];
    int status;
    user_info info;
    int lobby_idx;                  //0 ~ 19
    Lobby lobby_List[MAX_LOBBY];    //every lobby's information
    //Game game;
}PACKET;

//player char*
//type int 1개
//주사위 int 2개
//비용 int 1개
//칸 번호 int 1개
//빌딩 int 1개
//토지 소유 여부 int 1개
//남은 턴 int 1개

//game

/*
    send 

    -type 

    1024. 싱크 
        d[] : 플레이어 위치
        d[] : 플레이어 소유

    0. 주사위 입력 대기

    1. 주사위 던지기 
        server <- client    :   dice roll

        server: dice_roll   ->  ((0 ~ 5) + 1) * 2    -> d[0, 1]

        d[2] : 이동 후 칸 번호
        
        //echo server -> client

    2. 토지 매입 대기
        server <- client    :  

        d[0] : 칸 번호
        d[1] : 올라올 빌딩 - 비트마스크

        //echo server -> client

    3. 인수 여부 대기
        server <- client    : 

        d[0] : 칸 번호
        d[1] : 인수 가격

        //echo server -> client

    4. 비용 지불
        d[0] : 칸 번호
        d[1] : 비용

    5. 세계여행 여부 대기
        d[0] : 비용


    6. 세계여행 목적지 대기

    7. 대출 사용 여부
        d[0] : 칸 번호
        d[1] : 비용
    
    8. 매각 토지
        d[0] : 칸 번호
        d[1] : 비용
        d[2] : 토지 소유 여부

    9. 무인도 주사위 입력 대기
        d[0] : 남은 턴

    10. 무인도 주사위 결과
        d[0, 1] : 주사위


    11. 기부
        d[0] : 금액
    
    12. 기금 회수
        d[0] : 금액
    



*/


/*
recv
-type


*/