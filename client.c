#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncursesw/curses.h>    //in ubuntu: curses.h - -lncurses
//#include <ctime> //clock(), clock_t 
#include <time.h>
#include "packet.h"
#include <sys/time.h>
#include "game_clnt.h"
//#include "screen_handler.c"

enum states {LOGIN, LOBBY, ROOM, GAME};
int state = LOGIN;
int getlobbydata = NO;  //check client has earned lobby data from the server
//char input_c = '\0';    //user input at lobby state
//gcc client.c clnt.c game_clnt.h game_ui.c input.c marble.c -lncursesw -o client

//packet
PACKET packet;

//clients user information
user_info cur_user = {"", "", ""};

int pid;  //client pid

//lobby
int lobby_row = 1;

int sock;

//Functions
void init_client(int *sock);
void stop_wait(float s);

void sigint_handler(int signal, siginfo_t *siginfo, void *context);
void sigalrm_handler(int s);

void* screen_handler(void* arg);
void draw();
int login(int* arg);
void lobby(int* arg);

void game(int* arg);

//Main Function
int main(int argc, char *argv[]){

    pthread_t game_thread;
    void* result;
    //Client Socket
    int game_sock;
    init_client(&sock);
    login(&sock);
    printf("login success");
    setlocale(LC_CTYPE,"");
    initscr();
    clear();
    refresh();
    
    while(state != QUIT){
        //(2) lobby
        //game 끝나고 lobby 로 돌아가는 것 구현하기
        state = LOBBY;
        signal(SIGALRM, sigalrm_handler);
        lobby(&sock);

        //close(sock);  //maintain main server socket
        
        state = GAME;

        //(3) game

        game(&game_sock);

        //state = LOBBY;
        state = QUIT;
        //stop_wait(10);
    }

    state = QUIT;
    endwin();
    close(sock);
    free(result);
    
    return 0;
}

void init_client(int *sock){
    //signal
    struct sigaction handler;
    sigset_t blocked;

    struct sockaddr_in serv_addr;

    system("clear");

    //1. signal handling
    handler.sa_sigaction = sigint_handler;
    handler.sa_flags = SA_SIGINFO;

    sigemptyset(&blocked);
    sigaddset(&blocked, SIGQUIT);   //block just in case

    handler.sa_mask = blocked;

    if(sigaction(SIGINT, &handler, NULL) == -1)
        perror("sigaction() error!");
    signal(SIGQUIT, SIG_IGN);


    //3. socket
    *sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(SERVER_PORT);

    if(connect(*sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        perror("connect() error!");

    pid = getpid();
    write(*sock, &pid, sizeof(int));
}

//Closing Client
void sigint_handler(int signal, siginfo_t *siginfo, void *context){
    clear();
    //printf("Quit!\n");
    move(10, 10);
    addstr("Quit!");
    refresh();
    endwin();

    state = QUIT;       //for screen thread cancelation
    packet.result = QUIT;
    strcpy(packet.message,"QUIT");
    write(sock, &packet, sizeof(PACKET));
    close(sock);
    exit(0);
}

void stop_wait(float s){
    clock_t st;
    clock_t delay = CLOCKS_PER_SEC;

    st = clock();

    while (clock() - st < (delay * s)) 
    ;  

    return;
}

void draw(){
    if(state==LOBBY){
        int default_row = 10, default_col = 10;
        char lobbyscreen[MAX_LOBBY + 2][MAXLEN];
        strcpy(lobbyscreen[0], "**************************************\n");
        strcpy(lobbyscreen[21], "**************************************\n");
        char temp_str[300];
        int login_len = 10;
        
        //refresh screen
        clear();
        move(default_row, default_col);
        addstr(lobbyscreen[0]);

        for(int i = 1; i <= MAX_LOBBY; i++){
            if(i == lobby_row){
                standout();
            }

            sprintf(lobbyscreen[i],"* %2d | %8s                | %d/4 *\n", packet.lobby_List[i - 1].id, packet.lobby_List[i - 1].title, packet.lobby_List[i - 1].users);
            move(default_row + i, default_col);
            addstr(lobbyscreen[i]);

            if(i == lobby_row){
                standend();
            }
        }
        move(default_row + MAX_LOBBY + 1, default_col);
        addstr(lobbyscreen[21]);
        refresh();
    }
    else{
        clear();
        move(5, 5);
        addstr("in room.");
        refresh();
    }
}

void* screen_handler(void* arg){
    int sock = *(int*)arg;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);   //cancel immediately


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

    //sprintf(lobbyscreen[1],"*                                    *\n");

    while(state != QUIT && state!= GAME && state!=ROOM){    
            
            
    }

    return NULL;
}

int login(int* arg){

    int serv_sock = *arg;
    int readlen = 0;
    int *result;
    result = malloc(sizeof(int));


    while(state != QUIT){

        printf("ID : ");
        scanf("%s",packet.info.id);
        printf("Passwd : ");
        scanf("%s",packet.info.password);

        packet.result = 0;

        write(serv_sock, &packet, sizeof(PACKET));

        if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
            perror("read() error!");

        if(packet.result == SUCCESS){
            strcpy(cur_user.username, packet.info.username);
            result = packet.result;
            sleep(1);

            return result;
        }
        /*
        else if(packet.result == FAIL){
            
        }
        */
    }

    result = QUIT;    //QUIT
    return result;
}

int room(int serv_sock){

    int result;
    int readlen = 0;

    while(state != QUIT){       
        
       if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
            perror("read() error!");
        
        if(packet.result == SUCCESS){
            result = packet.result;

            //clear();
            //move(10, 10);
            //addstr("Let's start the game!");
            //stop_wait(0.5);
            //refresh();

            state = GAME;

            return result;
        }
    }

    return QUIT;
}

void lobby(int* arg){
    int lobbynum;
    int serv_sock = *arg;
    int readlen = 0;
    int *result;
    result = malloc(sizeof(int));
    struct itimerval new_timeset;
    int room(int serv_sock);
    
    new_timeset.it_interval.tv_sec = 2;
    new_timeset.it_interval.tv_usec = 0;
    new_timeset.it_value.tv_sec = 2;
    new_timeset.it_value.tv_usec = 0;

    setitimer(ITIMER_REAL, &new_timeset, NULL);

    packet.result = FAIL;

    //first get the lobby data
    packet.lobby_idx = -1;
    write(serv_sock, &packet, sizeof(PACKET));

    if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
        perror("read() error!");
    draw();
    //select lobby
    while(state != QUIT){
        
        //scanw("%d", &lobbynum);
        noecho();
        char key = getchar();//getch();
        echo();

        if(key == 'w'){
            if(lobby_row > 0)
                lobby_row -= 1;
            else
                lobby_row = 1;

            draw();
        }
        else if(key == 's'){
            if(lobby_row <= 20)
                lobby_row += 1;
            else
                lobby_row = 20;

            draw();
        }
        else if(key == '\r'){
            addstr("Enter!");
            refresh();
            packet.lobby_idx = (lobby_row - 1);
            write(serv_sock, &packet, sizeof(PACKET));

            if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
                perror("read() error!");

            draw();

            if(strcmp(packet.message, "Not accessible!\n") == 0){
                //printf("%s", packet.lobby.title);
            }
            else if(strcmp(packet.message, "Wrong Input\n") == 0){

            }
            else{
                signal(SIGALRM, SIG_IGN);
                clear();
                move(5, 5);
                addstr("in room.");
                refresh();
                *result = room(serv_sock);
                
                //  *result = SUCCESS;
                if(*result == SUCCESS){
                    stop_wait(0.05);
                    return (void*) result;
                }
            }
        }        
    }

    return NULL;
}

int check_if_different(int users[MAX_LOBBY], PACKET* pk){
    for(int i = 0;i<MAX_LOBBY;i++){
        if((pk->lobby_List[i].users) != users[i])
            return 1;
    }
    return 0;
}

void sigalrm_handler(int s){
    //packet.lobby_idx = -1;
    write(sock, &packet, sizeof(PACKET));

    static int prev_users[MAX_LOBBY];

    if((read(sock, &packet, sizeof(PACKET))) == -1)
        perror("read() error!");

    if (check_if_different(prev_users,&packet)){
        for(int i = 0;i<MAX_LOBBY;i++){
            prev_users[i] = packet.lobby_List[i].users;
        }
        draw();
    }

    return;
}


void game(int* arg){
    struct sockaddr_in game_addr;
    int *game_sock = arg;
    int readlen = 0;

    //new connection
    *game_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&game_addr, 0, sizeof(game_addr));
    game_addr.sin_family = AF_INET;
    game_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    game_addr.sin_port = htons(packet.lobby_List[packet.lobby_idx].port);

    //stop_wait(((rand() / 100) % 100));
    if(connect(*game_sock, (struct sockaddr*) &game_addr, sizeof(game_addr)) == -1)
        perror("connect() error!");

    while(state != QUIT){
        write(*game_sock, &packet, sizeof(PACKET));
        stop_wait(3);   //for accesible test

        mainLoop(*game_sock);
        if((readlen = read(*game_sock, &packet, sizeof(PACKET))) == -1)
            perror("read() error!");

        if(packet.result == QUIT){
            state = QUIT;
        }
    }

    return NULL;
}
