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

typedef struct{
	int dice[2];
    //add
}Game;

typedef struct{
    int result;
    char message[MAXLEN];
    int status;
    user_info info;
    int lobby_idx;
    Lobby lobby_List[MAX_LOBBY];
    Game game;
}PACKET;

PACKET packet;