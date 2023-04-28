#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>
//#include <ctime> //clock(), clock_t 
#include<time.h>


#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9190

#define MAX_LOBBY 20

#define MAXLEN 300

#define LOGIN   -1
#define LOBBY   -2
#define GAME    -3

int state = LOGIN;


#define FAIL    -10
#define SUCCESS 10
#define QUIT    -100

typedef struct{
    char id[MAXLEN];
    char password[MAXLEN];
    char username[MAXLEN];
}user_info;

typedef struct{
	char title[MAXLEN];
	int accessible;
	int num;
    int port;
}Lobby;

typedef struct{
	int dice[2];
    //add
}Game;

typedef struct{
    int result;
    char message[MAXLEN];
    user_info info;
    Lobby lobby;
    Game game;
}PACKET;

PACKET packet;

user_info cur_user = {"", "", ""};
//char username[MAXLEN];

//Client Socket
int sock;

clock_t delay = CLOCKS_PER_SEC;

//Functions
void sigint_handler(int signal, siginfo_t *siginfo, void *context);

void* screen_handler(void* arg);
void screen_init();
void* login(void* arg);
void* lobby(void* arg);

//Main Function
int main(int argc, char *argv[]){

    pthread_t screen_thread, login_thread, lobby_thread;
    void* result;

    //signal
    struct sigaction handler;
    sigset_t blocked;

    struct sockaddr_in serv_addr;

    //1. signal handling
    handler.sa_sigaction = sigint_handler;
    handler.sa_flags = SA_SIGINFO;

    sigemptyset(&blocked);
    sigaddset(&blocked, SIGQUIT);   //block just in case

    handler.sa_mask = blocked;

    if(sigaction(SIGINT, &handler, NULL) == -1)
        perror("sigaction() error!");
    signal(SIGQUIT, SIG_IGN);

    //2. ncurses
    initscr();
    clear();

    //3. socket
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(SERVER_PORT);

    if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        perror("connect() error!");

    //(0) Screen
    if(pthread_create(&screen_thread, NULL, screen_handler, (void*) &sock) != 0)
        perror("pthread_create() error!");

    state = LOGIN;

    //(1) login
    if(pthread_create(&login_thread, NULL, login, (void*) &sock) != 0)
        perror("pthread_create() error!");
    if(pthread_join(login_thread, &result) != 0)    //wait for login thread to end
        perror("pthread_join() error!"); 
    
    /*
    //changed to signal handling
    if(*(int*)result == QUIT){  //QUIT
        pthread_cancel(screen_thread);
        close(sock);
        printf("Client Quit!\n");
        return 0;
    }
    else if(*(int*)result == SUCCESS){
        //start lobby
        state = LOBBY;
        clear();
        printf("Lobby Screen!!\n");
    }
    */
    
    //game 끝나고 lobby 로 돌아가는 것 구현하기

    //(2) lobby
    if(pthread_create(&lobby_thread, NULL, lobby, (void*) &sock) != 0)
        perror("pthread_create() error!");
    if(pthread_join(lobby_thread, &result) != 0)    //wait for login thread to end
        perror("pthread_join() error!"); 

    /*
    //result will be port #
    
    close(sock);
    
    //new connect
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(*(int*) result);

    if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        perror("connect() error!");
    
    printf("New server %d\n", *(int*) result);
    */
    
    
    

    state = GAME;
    //(3) game

    state = QUIT;
    endwin();
    close(sock);
    free(result);
    return 0;
}

//Closing Client
void sigint_handler(int signal, siginfo_t *siginfo, void *context){
    clear();
    //printf("Quit!\n");
    endwin();

    state = QUIT;       //for screen thread cancelation
    packet.result = QUIT;
    strcpy(packet.message,"QUIT");
    write(sock, &packet, sizeof(PACKET));
    close(sock);
    exit(0);
}

void* screen_handler(void* arg){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);   //cancel immediately

    clock_t st;

    char *loginscreen[MAXLEN] = {
        "**************************************\n",
        "* [ Login ]                          *\n",
        "*            +--------------------+  *\n",
        "*   ID       |                    |  *\n", //+15
        "*            +--------------------+  *\n",
        "*                                    *\n",
        "*            +--------------------+  *\n",
        "*   Password |                    |  *\n",
        "*            +--------------------+  *\n",
        "**************************************\n"  //len 10
    };

    int login_len = 10;//strlen(loginscreen);  
    int id_point[] = {3, 15};
    int pw_point[] = {7, 15};

    char *lobbyscreen[MAXLEN] = {
        "**************************************\n",
        "*                                    *\n",
        "*                                    *\n",
        "**************************************\n"
    };

    //int lobby_len = strlen(loginscreen);  
    int lobby_row = 1;

    while(state != QUIT){
        switch(state){

            case LOGIN:
                //print login screen
                clear();
                move(10, 10);
                for(int i = 0; i < login_len; i++){
                    addstr(loginscreen[i]);
                    move((10 + (i + 1)), 10);
                }
                refresh();

                move(id_point[0] + 10, id_point[1] + 10);

                while(strcmp(packet.info.id, "") == 0)    //conditions
                    ;
                
                move(pw_point[0] + 10, pw_point[1] + 10);

                while(strcmp(packet.info.password, "") == 0)    //conditions
                    ;

                st = clock();

                while (clock() - st < delay) 
                ;  

                if(packet.result == SUCCESS){
                    state = LOBBY;

                    st = clock();
                    while (clock() - st < (delay * 4)) 
                    ;  
                }
                else if(packet.result == FAIL){
                    clear();
                    move(10, 10);
                    addstr(packet.message);
                    refresh();


                    st = clock();
                    while (clock() - st < (delay * 2)) 
                    ;  
                }
                break;

            case LOBBY:
                clear();
                move(10, 10);
                for(int i = 0; i < 4; i++){
                    addstr(lobbyscreen[i]);
                    move((10 + (i + 1)), 10);
                }
                refresh();

                st = clock();

                while (clock() - st < (delay * 3)) 
                ;  

                break;

            case GAME:
                break;
        }
    }

    return NULL;
}

void* login(void* arg){

    int serv_sock = *(int*) arg;
    int readlen = 0;
    int *result;
    result = malloc(sizeof(int));

    while(state != QUIT){

        strcpy(packet.info.id, "");
        strcpy(packet.info.password, "");

        scanw("%s", packet.info.id);
        scanw("%s", packet.info.password);

        /*
        if(strcmp(lpacket.id, "Q") == 0){
            *result = FAIL;
            write(serv_sock, &lpacket, sizeof(lpacket));    //quit 처리하기
            return (void*) result;
        }
        */

        packet.result = 0;

        write(serv_sock, &packet, sizeof(PACKET));

        if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
            perror("read() error!");

        if(packet.result == SUCCESS){
            strcpy(cur_user.username, packet.info.username);
            *result = packet.result;

            clear();
            move(10, 10);
            addstr("Welcome!");
            refresh();

            return (void*) result;
        }
        else if(packet.result == FAIL){
            /*
            clear();
            move(10, 10);
            addstr(packet.message);
            refresh();
            */
        }
    }

    *result = QUIT;    //QUIT
    return (void*) result;
}

void* lobby(void* arg){
    int lobbynum;
    int serv_sock = *(int*) arg;
    int readlen = 0;
    Lobby ypacket;
    int *result;
    result = malloc(sizeof(int));

    //print lobby list

    //select lobby

    while(state != QUIT){
        printf("Input Lobby number : ");
        scanf("%d", &lobbynum);

        if(lobbynum > 0 && lobbynum < MAX_LOBBY){
            sprintf(ypacket.title, "Room %d", lobbynum);
            write(serv_sock, &ypacket, sizeof(ypacket));

            if((readlen = read(serv_sock, &ypacket, sizeof(ypacket))) == -1)
                perror("read() error!");

            if(strcmp(ypacket.title, "Full!\n") == 0){
                printf("%s", ypacket.title);
            }
            else if(strcmp(ypacket.title, "Wrong Input\n") == 0){

            }
            else{
                //in room
                printf("In %s, num: %d, port: %d\n", ypacket.title, ypacket.num, ypacket.port);
                *result = SUCCESS;
                return (void*) result;
            }
            
        }
        else{
            printf("Wrong Input!\n");
        }
    }

    return NULL;
}