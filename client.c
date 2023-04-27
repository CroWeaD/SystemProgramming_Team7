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

/*
    Client 도 Quit signal로 구현하기
*/

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9190

#define MAX_LOBBY 20

#define MAXLEN 300

#define LOGIN   -1
#define LOBBY   -2
#define GAME    -3

typedef struct{
    int result;
    char message[MAXLEN];
    char id[MAXLEN];
    char password[MAXLEN];
}login_PACKET;

login_PACKET lpacket;

#define FAIL    -10
#define SUCCESS 10
#define QUIT    -100

typedef struct{
	char title[MAXLEN];
	int accesible;
	int num;
    const int port;
}Lobby;

int state = 0;

char username[MAXLEN];

void* screen_handler(void* arg);
void* login(void* arg);
void* lobby(void* arg);

int main(int argc, char *argv[]){

    pthread_t screen_thread, login_thread, lobby_thread;
    void* result;

    int sock;
    struct sockaddr_in serv_addr;

    //1. signal
    signal(SIGQUIT, SIG_IGN);

    //2. ncurses


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
    if(*(int*)result == FAIL){  //QUIT
        pthread_cancel(screen_thread);
        close(sock);
        printf("Client Quit!\n");
        return 0;
    }
    else if(*(int*)result == SUCCESS){
        //start lobby
        state = LOBBY;
        printf("Lobby Screen!!\n");
    }

    //game 끝나고 lobby 로 돌아가는 것 구현하기

    //(2) lobby
    if(pthread_create(&lobby_thread, NULL, lobby, (void*) &sock) != 0)
        perror("pthread_create() error!");
    if(pthread_join(lobby_thread, &result) != 0)    //wait for login thread to end
        perror("pthread_join() error!"); 

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

    state = GAME;
    //(3) game


    close(sock);
    free(result);
    return 0;
}

void* screen_handler(void* arg){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);   //cancel immediately

    //4. ncurses
    //initscr();
    //clear();

    while(1){
        switch(state){

            case LOGIN:
                break;
            case LOBBY:
                break;
            case GAME:
                break;
        }
    }

    return NULL;
}

void login_screen(){
    
}

void* login(void* arg){

    int serv_sock = *(int*) arg;
    int readlen = 0;
    int *result;
    result = malloc(sizeof(int));

    while(1){
        system("clear");
        printf("ID: ");
        scanf("%s", lpacket.id);
        printf("Password: ");
        scanf("%s", lpacket.password);

        if(strcmp(lpacket.id, "Q") == 0){
            *result = FAIL;
            write(serv_sock, &lpacket, sizeof(lpacket));    //quit 처리하기
            return (void*) result;
        }

        write(serv_sock, &lpacket, sizeof(lpacket));

        if((readlen = read(serv_sock, &lpacket, sizeof(lpacket))) == -1)
            perror("read() error!");

        if(lpacket.result == SUCCESS){
            strcpy(username, lpacket.message);
            *result = lpacket.result;

            printf("Wellcome %s!\n", username);
            sleep(3);

            return (void*) result;
        }
        else if(lpacket.result == FAIL){
            printf("%s", lpacket.message);
        }

        sleep(5);
    }

    //result = -1;    //QUIT
    //return result;
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

    while(1){
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
                printf("In %s, num: %d, port: %d, accessible: %d\n", ypacket.title, ypacket.num, ypacket.port, ypacket.accesible);
                *result = SUCCESS;
                return (void*) result;
            }
            
        }
        else{
            printf("Wrong Input!\n");
        }
    }



    

}