#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>    //in ubuntu: curses.h - -lncurses
//#include <ctime> //clock(), clock_t 
#include <time.h>
#include "packet.h"
#include <sys/time.h>
//#include "screen_handler.c"

enum states {LOGIN, LOBBY, ROOM, GAME};
int state = LOGIN;
int getlobbydata = NO;  //check client has earned lobby data from the server
//char input_c = '\0';    //user input at lobby state

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

void* login(void* arg);
void* lobby(void* arg);

void* game(void* arg);

//Main Function
int main(int argc, char *argv[]){

    pthread_t screen_thread, login_thread, lobby_thread, game_thread;
    void* result;
    //Client Socket
    int game_sock;

    init_client(&sock);

    //(0) Screen
    if(pthread_create(&screen_thread, NULL, screen_handler, (void*) &sock) != 0)
        perror("pthread_create() error!");

    //(1) login
    state = LOGIN;

    if(pthread_create(&login_thread, NULL, login, (void*) &sock) != 0)
        perror("pthread_create() error!");
    if(pthread_join(login_thread, &result) != 0)    //wait for login thread to end
        perror("pthread_join() error!"); 
    
    while(state != QUIT){
        //(2) lobby
        //game 끝나고 lobby 로 돌아가는 것 구현하기
        state = LOBBY;

        if(pthread_create(&lobby_thread, NULL, lobby, (void*) &sock) != 0)
            perror("pthread_create() error!");
        if(pthread_join(lobby_thread, &result) != 0)    //wait for login thread to end
            perror("pthread_join() error!"); 

        //close(sock);  //maintain main server socket
        
        state = GAME;

        //(3) game

        if(pthread_create(&game_thread, NULL, game, (void*) &game_sock) != 0)
            perror("pthread_create() error!");
        if(pthread_join(lobby_thread, &result) != 0)    //wait for login thread to end
            perror("pthread_join() error!"); 

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

    //2. ncurses
    initscr();
    clear();

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

void* screen_handler(void* arg){
    int sock = *(int*)arg;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);   //cancel immediately

    int default_row = 10, default_col = 10;

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

    char lobbyscreen[MAX_LOBBY + 2][MAXLEN];
    strcpy(lobbyscreen[0], "**************************************\n");
    strcpy(lobbyscreen[21], "**************************************\n");
    char temp_str[300];
    //sprintf(lobbyscreen[1],"*                                    *\n");

    while(state != QUIT){
        switch(state){
            case LOGIN:
                //print login screen
                clear();
                move(default_row, default_col);

                for(int i = 0; i < login_len; i++){
                    addstr(loginscreen[i]);
                    move((default_row  + (i + 1)), default_col);
                }
                stop_wait(0.05);
                refresh();

                move(default_row + id_point[0], default_col + id_point[1]);
                
                while(strcmp(packet.info.id, "") == 0)    //conditions
                    ;
                
                move(default_row + pw_point[0], default_col + pw_point[1]);
                noecho();
                while(strcmp(packet.info.password, "") == 0)    //conditions
                    ;
                echo();
                stop_wait(0.5);   //for sync

                if(packet.result == SUCCESS){
                    state = LOBBY;
                    clear();
                    move(default_row , default_col);
                    addstr("Welcome!");
                    refresh();
                    stop_wait(2);
                }
                else if(packet.result == FAIL){
                    clear();
                    move(default_row , default_col);
                    addstr(packet.message);
                    refresh();
                    stop_wait(2);
                }
                break;

            case LOBBY:
                //stop_wait(3);
                //get lobby info from the server
                if(getlobbydata == YES){
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

                    getlobbydata = NO;
                    stop_wait(0.05);
                    refresh();
                }

                if(packet.result == SUCCESS){
                    state = ROOM;
                }
                /*
                else if(packet.result == FAIL){
                    clear();
                    move(default_row, default_col);
                    addstr(packet.message);
                    refresh();
                    stop_wait(2);
                }
                */
                
                break;

            case ROOM:
                clear();
                sprintf(temp_str, "%s %d/4\n", packet.lobby_List[packet.lobby_idx].title, packet.lobby_List[packet.lobby_idx].users);
                move(default_row, default_col);
                addstr(temp_str);
                stop_wait(0.05);
                refresh();
                
                break;

            case GAME:
                //stop_wait(2); 
                //clear();
                //sprintf(temp_str, "New server %d\n", packet.lobby_List[lobby_row].port);
                //addstr(temp_str);
                //refresh();
                //stop_wait(2);
                stop_wait(1); 
                char temp_str[MAXLEN];
                clear();
                sprintf(temp_str, "%s %d/4\n", packet.lobby_List[packet.lobby_idx].title, packet.lobby_List[packet.lobby_idx].users);
                move(13, 10);
                addstr(temp_str);
                stop_wait(4);
                refresh();
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

        packet.result = 0;

        write(serv_sock, &packet, sizeof(PACKET));

        if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
            perror("read() error!");

        if(packet.result == SUCCESS){
            strcpy(cur_user.username, packet.info.username);
            *result = packet.result;
            stop_wait(2);

            return (void*) result;
        }
        /*
        else if(packet.result == FAIL){
            
        }
        */
    }

    *result = QUIT;    //QUIT
    return (void*) result;
}

int room(int serv_sock){

    int result;
    int readlen = 0;

    while(state != QUIT){
       
       /*
       noecho();
        char key = getchar();//getch();
        echo();

        if(key == 'r' || key == 'R'){
            addstr("Ready");
            refresh();
            strcpy(packet.message, "Ready");

            write(serv_sock, &packet, sizeof(PACKET));

            if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
                perror("read() error!");
        }
       */
        
        
       if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
            perror("read() error!");
        
        if(packet.result == SUCCESS){
            result = packet.result;

            clear();
            move(10, 10);
            addstr("Let's start the game!");
            stop_wait(0.5);
            refresh();

            state = GAME;

            return result;
        }
    }

    return QUIT;
}

void* lobby(void* arg){
    int lobbynum;
    int serv_sock = *(int*) arg;
    int readlen = 0;
    int *result;
    result = malloc(sizeof(int));
    struct itimerval new_timeset;
    int room(int serv_sock);

    signal(SIGALRM, sigalrm_handler);
    
    new_timeset.it_interval.tv_sec = 5;
    new_timeset.it_interval.tv_usec = 0;
    new_timeset.it_value.tv_sec = 5;
    new_timeset.it_value.tv_usec = 0;

    setitimer(ITIMER_REAL, &new_timeset, NULL);

    packet.result = FAIL;

    //first get the lobby data
    packet.lobby_idx = -1;
    write(serv_sock, &packet, sizeof(PACKET));

    if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
        perror("read() error!");

    getlobbydata = YES;

    stop_wait(0.5);

    //select lobby
    while(state != QUIT){
        
        //scanw("%d", &lobbynum);
        noecho();
        char key = getchar();//getch();
        echo();

        if(key == 'w'){
            //Up
            //addstr("Up");
            //refresh();
            if(lobby_row > 0)
                lobby_row -= 1;
            else
                lobby_row = 1;

            getlobbydata = YES;
        }
        else if(key == 's'){
            //Down
            //addstr("Down");
            //refresh();
            if(lobby_row <= 20)
                lobby_row += 1;
            else
                lobby_row = 20;

            getlobbydata = YES;
        }
        else if(key == '\r'){
            addstr("Enter!");
            refresh();
            packet.lobby_idx = (lobby_row - 1);
            write(serv_sock, &packet, sizeof(PACKET));

            if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
                perror("read() error!");

            getlobbydata = YES;

            if(strcmp(packet.message, "Not accessible!\n") == 0){
                //printf("%s", packet.lobby.title);
            }
            else if(strcmp(packet.message, "Wrong Input\n") == 0){

            }
            else{
                //in room
                //printf("In %s, num: %d, port: %d\n", packet.lobby.title, packet.lobby.users, packet.lobby.port);
                //clear();
                //addstr(packet.lobby_List[lobby_row].title);
                //refresh();

                *result = room(serv_sock);
                
                //  *result = SUCCESS;
                if(*result == SUCCESS)
                    return (void*) result;
            }

            stop_wait(0.05);
        }        
    }

    return NULL;
}

void sigalrm_handler(int s){
    //packet.lobby_idx = -1;
    write(sock, &packet, sizeof(PACKET));

    if((read(sock, &packet, sizeof(PACKET))) == -1)
        perror("read() error!");

    getlobbydata = YES;

    return;
}

void* game(void* arg){
    struct sockaddr_in game_addr;
    int *game_sock = (int*)arg;
    int readlen = 0;

    //new connection
    *game_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&game_addr, 0, sizeof(game_addr));
    game_addr.sin_family = AF_INET;
    game_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    game_addr.sin_port = htons(packet.lobby_List[packet.lobby_idx].port);

    stop_wait((2 + rand() % 4) / 2);
    //stop_wait(3);

    if(connect(*game_sock, (struct sockaddr*) &game_addr, sizeof(game_addr)) == -1)
        perror("connect() error!");

    while(state != QUIT){
        write(*game_sock, &packet, sizeof(PACKET));

        if((readlen = read(*game_sock, &packet, sizeof(PACKET))) == -1)
            perror("read() error!");

        if(packet.result == QUIT){
            state = QUIT;
        }
    }

    return NULL;
}
