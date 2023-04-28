#include <stdio.h>
#include <mysql.h>      //gcc option $(mysql_config --libs --cflags)
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

/*
    Signal로 에러 발생시 수행할 행동을 설정해야겠음.
    SIGFPE
*/

#define PORT 9190

#define MAX_LOBBY 20
#define MAX_CLNT MAX_LOBBY * 4

#define MAXLEN 300

//#define SCREEN_ROW 200
//#define SCREEN_COL 200
//#define MAX_ROW 1024

int clnt_cnt = 0;
//int clnt_socks[MAX_CLNT];

typedef struct{
    char username[MAXLEN];
    int socket;
    int pid;
}Client;

Client clnt_list[MAX_CLNT];

//essencial
char *server = "localhost";  //server path
char *user = "root";    //user name
char *password = "sinanju.237"; //password of the user

//optional
char *database = "BlueMarble";  //DataBase name
char *userTable = "user_tb";

char *query = "select * from user_tb";    //query

MYSQL *conn = NULL;    //variable for mysql connection  
MYSQL_RES *res = NULL;  //variable for result of query  
MYSQL_ROW row;   //real data which result from query

//mutex
pthread_mutex_t mutex;

//socket
int serv_sock, clnt_sock;

//Packet
/* Values for result */
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
//Lobby lobby_list[20];

//Functions
void sigint_handler(int signal, siginfo_t *siginfo, void *context);
void client_close(int clnt_socket);

void *clnt_main(void* clnt_socket);

int login(int clnt_socket, PACKET* packet_ptr);
int check_duplication(char *username);

void lobby(int clnt_socket, PACKET* packet_ptr);
void room(int clnt_socket, int room);


/* Main function */
int main(int argc, char *argv[]){

    //socket
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;    //socklen_t == int
    //mutex
    pthread_t t_id;
    //signal
    struct sigaction handler;
    sigset_t blocked;
    int pid = getpid(); //for evoke signal
   
    system("clear");

    //0. mysql
    if((conn = mysql_init(NULL)) == NULL)
        perror("mysql_init() error!");
    
    if(mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL){
        mysql_close(conn);
        perror("mysql_real_connect() error!");
    }

    //init settings
    pthread_mutex_init(&mutex, NULL);   //1. mutex

    handler.sa_sigaction = sigint_handler;  //2. signal handling
    handler.sa_flags = SA_SIGINFO;

    sigemptyset(&blocked);
    sigaddset(&blocked, SIGQUIT);   //block just in case

    handler.sa_mask = blocked;

    if(sigaction(SIGINT, &handler, NULL) == -1)
        perror("sigaction() error!");
    signal(SIGQUIT, SIG_IGN);   //ignore sigquit

    //3. socket setting
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(PORT);

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1){
        perror("bind() error!");
        exit(1);
    }
        
    if(listen(serv_sock, 5) == -1){
        perror("listen() error!");
        exit(1);
    }

    //Debug Message
    printf("Server On!!\n");

    while(1){
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, (socklen_t*) &clnt_adr_sz);

        pthread_mutex_lock(&mutex);
        clnt_list[clnt_cnt].pid = -1;
        clnt_list[clnt_cnt].socket = clnt_sock;
        strcpy(clnt_list[clnt_cnt++].username, "");
        //clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id, NULL, clnt_main, (void*) &clnt_sock);
        pthread_detach(t_id);

        printf("Log: connected client IP[%s] - Port[%d]\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
    }

    //Just in case - results error
    //mysql_free_result(res);     //free result
    //mysql_close(conn);    //close connection
    //close(serv_sock);  
    return 0;
}

//Closing Server
void sigint_handler(int signal, siginfo_t *siginfo, void *context){
    system("clear");

    printf("Turnning off the server...\n");

    //close every socket that is connected to server
    while(clnt_cnt != -1){
        pthread_mutex_lock(&mutex);
        printf("Closing client %s - clnt_socks[%d]: %d\n", clnt_list[clnt_cnt].username, clnt_cnt, clnt_list[clnt_cnt].socket/*clnt_socks[clnt_cnt]*/);
        close(clnt_list[clnt_cnt--].socket /*clnt_socks[clnt_cnt--]*/);
        pthread_mutex_unlock(&mutex);
    }

    if(res != NULL)
        mysql_free_result(res);     //free result
    if(conn != NULL)
        mysql_close(conn);    //close connection

    close(serv_sock); 
    printf("\nServer Off\n");
    exit(0);
}

//Close a client
void client_close(int clnt_socket){

    pthread_mutex_lock(&mutex);
    for(int i = 0; i < clnt_cnt; i++){
        if(clnt_socket == clnt_list[i].socket /*clnt_socket == clnt_socks[i]*/){
            printf("Closing client: %d\n", clnt_list[i].socket /*clnt_socks[i]*/);

            while(i < clnt_cnt){
                clnt_list[i].socket = clnt_list[i + 1].socket;
                strcpy(clnt_list[i].username, clnt_list[i + 1].username);
                clnt_list[i].pid = clnt_list[i + 1].pid;
                //clnt_socks[i] = clnt_socks[i + 1];
                i += 1;
            }
            break;
        }
    }

    clnt_cnt -= 1;
    pthread_mutex_unlock(&mutex);
    close(clnt_socket);

    return;
}

//Client Thread Main function
void *clnt_main(void* clnt_socket){
    int clnt_sock = *(int*) clnt_socket;
    PACKET packet;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    //1. Login
    if(login(clnt_sock, &packet) == QUIT)
        return NULL;
    
    //2. Lobby
    lobby(clnt_sock, &packet);
    //room(clnt_sock, 10);    //debuggin

    //3. Game

    if(packet.result == QUIT){
        client_close(clnt_sock);
    }

    return NULL;
}

int login(int clnt_socket, PACKET* packet_ptr){
    
    int readlen = 0;

    query = "SELECT * FROM user_tb";

    while(1){

        //get input
        if((readlen = read(clnt_socket, packet_ptr, sizeof(PACKET))) == -1)
            perror("read() error!");

        //Client Quit
        if((packet_ptr->result == QUIT) && (strcmp(packet_ptr->message, "QUIT") == 0)){
            client_close(clnt_socket);
            return QUIT;
        }
        //check
        //id: 1, password: 2, username: 3
        if(mysql_query(conn, query)){
            mysql_close(conn);
            perror("mysql_query() error!");
        }
        else{
            res = mysql_use_result(conn);   //MYSQL *mysql_use_result(MYSQL *conn);    -> after query, use this fuction to get result
            int fields = mysql_num_fields(res);
            
            while((row = mysql_fetch_row(res))){   //MYSQL_ROW *mysql_fetch_row(MYSQL_RES *res)    -> get a row from the result
                printf("%s %s %s\n", row[1], row[2], row[3]);
                if(strcmp(row[1], packet_ptr->info.id) == 0){
                    if(strcmp(row[2], packet_ptr->info.password) == 0){
                        //valid ID, password
                        if(check_duplication(row[3]) == 0){
                            strcpy(packet_ptr->info.username, row[3]);
                            packet_ptr->result = SUCCESS;
                            printf("User %s, login success!\n", row[3]);

                            if(write(clnt_socket, packet_ptr, sizeof(PACKET)) != readlen)
                                perror("write() error!");

                            mysql_free_result(res);
                            return SUCCESS;
                        }
                        else if(check_duplication(row[3]) == 1){
                            packet_ptr->result = FAIL;
                            strcpy(packet_ptr->message, "The user is already logged in.\n");
                        }
                    }
                }
                else{
                    strcpy(packet_ptr->message, "Wrong ID or Password. Please try again.\n");
                    packet_ptr->result = FAIL;
                }
            }

            mysql_free_result(res);
            printf("Login Failed!\n");
            printf("%s %s %d\n", packet_ptr->message, packet_ptr->info.username, packet_ptr->result);
            if(write(clnt_socket, packet_ptr, sizeof(PACKET)) != readlen)
                perror("write() error!");
        }
    }
}

//login duplication check
int check_duplication(char *username){

    for(int i = 0; i < clnt_cnt; i++){
        if(strcmp(username, clnt_list[i].username) == 0){
            return 1;   //Duplication
        }
    }

    pthread_mutex_lock(&mutex);
    strcpy(clnt_list[clnt_cnt].username, username);
    clnt_cnt += 1;
    pthread_mutex_unlock(&mutex);
    
    return 0;
}

void lobby(int clnt_socket, PACKET* packet_ptr){

    int readlen = 0;
    int i;
    Lobby ypacket;

    /*
    while(1){
        if((readlen = read(clnt_socket, &ypacket, sizeof(ypacket))) == -1)
                perror("read() error!");

        for(i = 0; i < MAX_LOBBY; i++){
            if(strcmp(ypacket.title, lobby_list[i].title) == 0){
                if(lobby_list[i].accessible == 1){
                    
                    ypacket.port = lobby_list[i].port;
                    strcpy(ypacket.title, lobby_list[i].title);

                    lobby_list[i].num += 1;
                    ypacket.num = lobby_list[i].num;

                    if(lobby_list[i].num == 4){
                        lobby_list[i].accessible = 0;
                    }

                    ypacket.accessible = lobby_list[i].accessible;

                    if(write(clnt_socket, &ypacket, sizeof(ypacket)) != readlen)
                        perror("write() error!");

                    return ;

                }
                else{
                    strcpy(ypacket.title, "Full!\n");
                    ypacket.accessible = lobby_list[i].accessible;
                }
            }
        }
        
        if(i == MAX_LOBBY){
            strcpy(ypacket.title, "Wrong Input\n");
        }

        if(write(clnt_socket, &ypacket, sizeof(ypacket)) != readlen)
            perror("write() error!");
    }
    */
    
    
}

void room(int clnt_socket, int room){

    //send current room state

    int new_serv_sock;
    struct sockaddr_in new_serv_adr, clnt_adr;
    int clnt_adr_sz = sizeof(clnt_adr);

    pid_t pid;

    pid = fork();

    if(pid == 0){
        //game
        new_serv_sock = socket(PF_INET, SOCK_STREAM, 0);

        memset(&new_serv_adr, 0, sizeof(new_serv_adr));
        new_serv_adr.sin_family = AF_INET;
        new_serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        //new_serv_adr.sin_port = htons(lobby_list[room - 1].port);

        if(bind(serv_sock, (struct sockaddr*) &new_serv_adr, sizeof(new_serv_adr)) == -1){
            perror("bind() error!");
            exit(1);
        }
            
        if(listen(serv_sock, 5) == -1){
            perror("listen() error!");
            exit(1);
        }

        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, (socklen_t*) &clnt_adr_sz);

        printf("New server!!\n");
    }

}

void game(int clnt_socket){

}
