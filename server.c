#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "game.h"
#include <mysql.h>      //gcc option $(mysql_config --libs --cflags)
//gcc server.c -o server -I/opt/homebrew/Cellar/mysql/8.0.33/include/mysql -L/opt/homebrew/Cellar/mysql/8.0.33/lib -lmysqlclient -L/opt/homebrew/opt/openssl@1.1/lib -lssl -lcrypto -lresolv
/*
    mysql 8.0.33 version has -lzlib -lzstd linking problem
*/

// gcc server.c game.c gamePacket.c gamePacket.h -o server $(mysql_config --cflags --libs)

#include "packet.h"
//#include "server_mysql.c"
//#include "login.c"
//#include "server_signal.c"
//#include "socket.c"

#define MAX_CLNT MAX_LOBBY * 4

//client struct 수정
typedef struct{
    char username[MAXLEN];
    int socket;
    int pid;
}Client;

Client clnt_list[MAX_CLNT];
int clnt_cnt = 0;

//mutex
pthread_mutex_t mutex;

//MySQL
MYSQL *conn;    //variable for mysql connection  

//socket
int serv_sock;

//Lobby
Lobby lobby_list[MAX_LOBBY];
int lobby_client_sock[MAX_LOBBY][4] = {-1, };
int ready[MAX_LOBBY][4] = {NO, };
//int game_on_socket[MAX_LOBBY] = {0};  /accessible 로 처리

//fork
pid_t game_pids[MAX_LOBBY];  

int main_process = 1;    //True

//Functions
//Init Blue Marble server
void init_server();
void mysql_initialization();
void signal_init();
void server_socket_init();
void lobby_init();

//Closing server, client
void sigint_handler(int signal, siginfo_t *siginfo, void *context);
void client_close(int clnt_socket);
void kill_game();

//Client main
void *clnt_main(void* clnt_socket);

//Login
int login(int clnt_socket, PACKET* packet_ptr);
int check_duplication(int clnt_sock, char *username);

//Lobby
int lobby(int clnt_socket, PACKET* packet_ptr);
int room(int clnt_socket, PACKET* packet_ptr);

//Game
void game(int clnt_socket, PACKET* packet_ptr);

void *game_maker(void* arg);
void sigterm_handler(int signal, siginfo_t *siginfo, void *context);


MYSQL_RES * mysql_do_query(char *query);

/* Main function */
int main(int argc, char *argv[]){
    //socket
    int clnt_sock;
    struct sockaddr_in clnt_adr;
    int clnt_adr_sz;    //socklen_t == int
    //mutex
    pthread_t t_id;

    init_server();

    //Game maker
    pthread_create(&t_id, NULL, game_maker, (void*) NULL);
    pthread_detach(t_id);

    //Debug Message
    printf("Server On!!\n");

    if(main_process == 1){
        while(1){
            clnt_adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, (socklen_t*) &clnt_adr_sz);

            pthread_mutex_lock(&mutex);
            clnt_list[clnt_cnt].pid = -1;
            clnt_list[clnt_cnt].socket = clnt_sock;
            //get pid
            read(clnt_sock, &clnt_list[clnt_cnt].pid, sizeof(int));
            strcpy(clnt_list[clnt_cnt].username, "");
            //clnt_socks[clnt_cnt++] = clnt_sock;
            clnt_cnt += 1;
            pthread_mutex_unlock(&mutex);

            pthread_create(&t_id, NULL, clnt_main, (void*) &clnt_sock);
            pthread_detach(t_id);

            printf("Log: connected client %s : %d, PID[%d]\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock, clnt_list[clnt_cnt - 1].pid);
        }
    }
    

    return 0;
}

void init_server(){

    system("clear");

    //0. MySQL
    mysql_initialization();

    //1. mutex
    pthread_mutex_init(&mutex, NULL);  

    //2. signal handling
    signal_init();

    //3. socket setting
    server_socket_init();

    //Init lobby data
    lobby_init();

    //init client list
    for(int i = 0; i < MAX_CLNT; i++){
        clnt_list[i].pid = 0;
        clnt_list[i].socket = 0;
        strcpy(clnt_list[i].username,"");
    }

    return;
}

void server_socket_init(){
    //socket
    struct sockaddr_in serv_adr;
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(SERVER_PORT);
    int optval = 1;
    setsockopt(serv_sock,0,SO_REUSEADDR,&optval,sizeof(optval));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1){
        perror("bind() error!");
        exit(1);
    }
        
    if(listen(serv_sock, 5) == -1){
        perror("listen() error!");
        exit(1);
    }
}

void signal_init(){
    //signal
    struct sigaction handler;
    sigset_t blocked;

    //2. signal handling
    handler.sa_sigaction = sigint_handler;  
    handler.sa_flags = SA_SIGINFO;

    sigemptyset(&blocked);
    sigaddset(&blocked, SIGQUIT);   //block just in case

    handler.sa_mask = blocked;

    if(sigaction(SIGINT, &handler, NULL) == -1)
        perror("sigaction() error!");

    signal(SIGQUIT, SIG_IGN);   //ignore sigquit
}

void mysql_initialization(){
    printf("enter mysql-server password:");
    char buf[100];
    scanf("%s",buf);
    //essencial
    char *server = "localhost";  //server path
    char *user = "root";    //user name
    char *password = "1q2w3e4r@@"; //password of the user

    //optional
    char *database = "BlueMarble";  //DataBase name
    //char *userTable = "user_tb";

    //0. mysql
    if((conn = mysql_init(NULL)) == NULL)
        printf("%s\n",mysql_error(conn));
    
    if(mysql_real_connect(conn, server, user, buf, database, 0, NULL, 0) == NULL){
        mysql_close(conn);
        printf("%s\n",mysql_error(conn));
    }
}

MYSQL_RES * mysql_do_query(char *query){

    //printf("%s\n", query);
    if(mysql_query(conn, query)){
        mysql_close(conn);
        printf("%s\n",mysql_error(conn));
    }

    //res = mysql_use_result(conn);   //MYSQL *mysql_use_result(MYSQL *conn);    -> after query, use this fuction to get result
    //int fields = mysql_num_fields(res);

    return mysql_use_result(conn);    //use    mysql_free_result(res);     //free result after mysql_do_query();
}

void lobby_init(){
    //MySQL
    MYSQL_RES *res = NULL;  //variable for result of query  //change to local variable  
    MYSQL_ROW row;   //real data which result from query

    res = mysql_do_query("SELECT * FROM lobby");

    //id: 0, title: 1, access: 2, users: 3
    //printf("Init lobby data\n");
    while((row = mysql_fetch_row(res))){
        //printf("%s %s %s %s\n", row[0], row[1], row[2], row[3]);
        int idx = atoi(row[0]) - 1;
        //printf("idx : %d", idx);
    
        lobby_list[idx].id = atoi(row[0]);
        strcpy(lobby_list[idx].title, row[1]);
        lobby_list[idx].accessible = atoi(row[2]);
        lobby_list[idx].users = atoi(row[3]);
        lobby_list[idx].port = SERVER_PORT + idx + 1;
        //printf("%d %s %d %d %d\n", lobby_list[idx].id, lobby_list[idx].title, lobby_list[idx].accessible, lobby_list[idx].users, lobby_list[idx].port);
    }

    mysql_free_result(res);     //free result
}

void kill_game(){
    for(int i = 0; i < MAX_LOBBY; i++){
        kill(game_pids[i], SIGKILL);
    }
}

//Closing Server
void sigint_handler(int signal, siginfo_t *siginfo, void *context){
    system("clear");

    printf("Turnning off the server...\n");

    //close every socket that is connected to server
    for(int i = (clnt_cnt - 1); i >= 0; i--){
        client_close(clnt_list[i].socket);
    }

    kill_game();

    /*
    if(res != NULL)
        mysql_free_result(res);     //free result
    */
    mysql_close(conn);    //close connection

    close(serv_sock); 
    printf("\nServer Off\n");
    exit(0);
}

void sigterm_handler(int signal, siginfo_t *siginfo, void *context){
    //printf("Turnning off the Game...\n");

    //close every socket that is connected to server
    //for(int i = (clnt_cnt - 1); i >= 0; i--){
    //    client_close(clnt_list[i].socket);
    //}

    /*
    if(res != NULL)
        mysql_free_result(res);     //free result
    */
}

//Close a client
void client_close(int clnt_socket){
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < clnt_cnt; i++){
        if(clnt_socket == clnt_list[i].socket /*clnt_socket == clnt_socks[i]*/){
            printf("Closing client %s - clnt_socks[%d]: %d\n", clnt_list[i].username, i, clnt_list[i].socket/*clnt_socks[clnt_cnt]*/);

            //PACKET temp_pkt;
            //temp_pkt.result = QUIT;
            //write(clnt_socket, &temp_pkt, sizeof(PACKET));

            while(i < clnt_cnt){
                clnt_list[i].socket = clnt_list[i + 1].socket;
                strcpy(clnt_list[i].username, clnt_list[i + 1].username);
                clnt_list[i].pid = clnt_list[i + 1].pid;
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

void clear_lobby(int lobby_idx){
    //lobby list 수정
    
    lobby_list[lobby_idx].accessible = 1;
    lobby_list[lobby_idx].users = 0;
}

void *game_maker(void* arg){
    //arg
    void *argv = arg;
    
    //Pipe
    int thepipe[2];     //pipes[MAX_LOBBY][2];

    int result;
    
    //Socket
    int new_serv, new_clnt_sock;
    struct sockaddr_in new_serv_adr, new_clnt_adr;
    int clnt_adr_sz = sizeof(new_clnt_adr);
    int game_clnt_list[4] = {-1, };

    //signal
    struct sigaction handler;
    sigset_t blocked;

    ///Packet
    PACKET packet;

    printf("Game Maker Thread\n");

    pipe(thepipe);

    int room_id = 0;

    for(room_id = 0; room_id < MAX_LOBBY; room_id++){

        game_pids[room_id] = fork();

        if(game_pids[room_id] == 0){
            main_process = 0;  //False
            printf("room_id: %d\n", room_id);   //Debugging
            close(thepipe[0]);  //close read end
            break;
        }

        else{
            continue;
        }
    }

    if(game_pids[room_id] == 0){  
        char clnt_names[4][300];
        //SIGTERM handling 
        handler.sa_sigaction = sigterm_handler;  
        handler.sa_flags = SA_SIGINFO;

        sigemptyset(&blocked);
        sigaddset(&blocked, SIGINT);   //block just in case

        handler.sa_mask = blocked;

        if(sigaction(SIGTERM, &handler, NULL) == -1)    //만약 client 이외의 경로에서 데이터를 받아야 할 때 대비
            perror("sigaction() error!");

        signal(SIGINT, SIG_IGN);    //SIGKILL로만 죽게 함.
        
        //Open Socket
        new_serv = socket(PF_INET, SOCK_STREAM, 0);

        memset(&new_serv_adr, 0, sizeof(new_serv_adr));
        new_serv_adr.sin_family = AF_INET;
        new_serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        new_serv_adr.sin_port = htons(lobby_list[room_id].port);

        if(bind(new_serv, (struct sockaddr*) &new_serv_adr, sizeof(new_serv_adr)) == -1){
            perror("bind() error!");
            exit(1);
        }
            
        if(listen(new_serv, 5) == -1){
            perror("listen() error!");
            exit(1);
        }

        while(1){
            int cnt = 0;

            while (1){
                clnt_adr_sz = sizeof(new_clnt_adr);
                game_clnt_list[cnt++] = accept(new_serv, (struct sockaddr*) &new_clnt_adr, (socklen_t*) &clnt_adr_sz);
                printf("Log: Game connected client %s : %d\n", inet_ntoa(new_clnt_adr.sin_addr), game_clnt_list[cnt-1]);
                //cnt += 1;

                if(cnt == 1){
                    read(game_clnt_list[0], &packet, sizeof(PACKET));    //read data from the first client
                    //lobby.user ...
                    printf("Lobby[%d] user: %d\n", packet.lobby_idx + 1, packet.lobby_List[room_id].users);
                    strcpy(clnt_names[cnt - 1], packet.info.username);
                }

                printf("cnt: %d, %d\n", cnt, game_clnt_list[cnt-1]);
                read(game_clnt_list[cnt - 1], &packet, sizeof(PACKET));   //garbage
                strcpy(clnt_names[cnt - 1], packet.info.username);
                
                if(cnt == packet.lobby_List[room_id].users){
                    break;
                }
                
                /*
                if(cnt == 4){
                    break;
                }
                */
                
                //printf("@\n");
                fflush(stdout);
            }
            /*
            for(int i = 1; i < cnt; i++){
                read(game_clnt_list[i], &packet, sizeof(PACKET));   //garbage
            }
            */
            
  
            //Run Game

            printf("Game!\n");
            fflush(stdout);

            start_game(cnt,game_clnt_list, clnt_names);
            //game();
            //Debug

            //게임 종료

            for(int i = 0; i < cnt; i++){
                packet.result = QUIT;
                write(game_clnt_list[i], &packet, sizeof(PACKET));
            }

            clear_lobby(room_id);

            //close sockets
            for(int i = 0; i < 4; i++){
                if(game_clnt_list[i] != -1)
                    close(game_clnt_list[i]);
            }

            printf("write pipe\n");
            write(thepipe[1], &room_id, sizeof(int)); 
        }
    }
    else{
        close(thepipe[1]);  //close write end
        //read pipe input
        while(1){
            read(thepipe[0], &result, sizeof(int));

            printf("Result: %d\n", result);

            //close clients
            for(int i = 0; i < 4; i++)
                lobby_client_sock[result][i] = -1;
            
           clear_lobby(result);
        }
        
    }

    return NULL;
}

//Client Thread Main function
void *clnt_main(void* clnt_socket){
    int clnt_sock = *(int*) clnt_socket;
    PACKET packet;
    int result;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    //1. Login
    if(login(clnt_sock, &packet) == QUIT){
        client_close(clnt_sock);
        return NULL;
    }
    else{
            result = lobby(clnt_sock, &packet);

            if(result == QUIT){
                client_close(clnt_sock);
                return NULL;
            }
            else if(result == SUCCESS){
                result = room(clnt_sock, &packet);
            }         
    }
    
    /*
    if(packet.result == QUIT){
        client_close(clnt_sock);
    }
    */
    

    return NULL;
}

int login(int clnt_socket, PACKET* packet_ptr){

    MYSQL_RES *res = NULL;  //variable for result of query  
    MYSQL_ROW row;   //real data which result from query
    char *query;    //query
    
    int readlen = 0;

    query = "SELECT * FROM user_tb";

    while(1){

        memset(packet_ptr->message, 0, MAXLEN); //clear packet message
        //get input
        if((readlen = read(clnt_socket, packet_ptr, sizeof(PACKET))) == -1)
            perror("read() error!");

        //Client Quit
        if((packet_ptr->result == QUIT) && (strcmp(packet_ptr->message, "QUIT") == 0)){
            client_close(clnt_socket);
            return QUIT;
        }

        if(strcmp("LOGIN", packet_ptr->info.username) == 0){
            //printf("ID: %s, PASSWORD: %s\n", packet_ptr->info.id, packet_ptr->info.password); //Debugging
            //check
            //id: 1, password: 2, username: 3
            res = mysql_do_query(query);
                
            while((row = mysql_fetch_row(res))){   //MYSQL_ROW *mysql_fetch_row(MYSQL_RES *res)    -> get a row from the result
                //printf("%s %s %s\n", row[1], row[2], row[3]);
                if(strcmp(row[1], packet_ptr->info.id) == 0){
                    if(strcmp(row[2], packet_ptr->info.password) == 0){
                        //valid ID, password
                        if(check_duplication(clnt_socket, row[3]) == 0){
                            strcpy(packet_ptr->info.username, row[3]);
                            packet_ptr->result = SUCCESS;
                            printf("User %s, login success!\n", row[3]);

                            if(write(clnt_socket, packet_ptr, sizeof(PACKET)) != readlen)
                                perror("write() error!");

                            mysql_free_result(res);
                            return SUCCESS;
                        }
                        else if(check_duplication(clnt_socket, row[3]) == 1){
                            packet_ptr->result = FAIL;
                            strcpy(packet_ptr->message, "The user is already logged in.\n");
                            break;
                        }
                    }
                }
                else{
                    strcpy(packet_ptr->message, "Wrong ID or Password. Please try again.\n");
                    packet_ptr->result = FAIL;
                }
            }

            mysql_free_result(res);
            res = NULL;
            printf("Login Failed!\n");
            printf("%s %s %d\n", packet_ptr->message, packet_ptr->info.username, packet_ptr->result);

            if(write(clnt_socket, packet_ptr, sizeof(PACKET)) != readlen)
                perror("write() error!");
        }
        else{
            //Sign up
            //id: 1, password: 2, username: 3
            res = mysql_do_query(query);
            
            while((row = mysql_fetch_row(res))){   //MYSQL_ROW *mysql_fetch_row(MYSQL_RES *res)    -> get a row from the result
                //printf("%s %s %s\n", row[1], row[2], row[3]);
                if(strcmp(row[3], packet_ptr->info.username) == 0){
                    if(strcmp(row[2], packet_ptr->info.password) == 0 && strcmp(row[1], packet_ptr->info.id) == 0){
                        packet_ptr->result = FAIL;
                        break;
                    }
                    else{
                        char temp[300];
                        sprintf(temp, "INSERT INTO user_tb (userid, userpw, username) values ('%s', '%s', '%s')", packet_ptr->info.id, packet_ptr->info.password, packet_ptr->info.username);
                        query = temp;
                        mysql_do_query(query);
                        packet_ptr->result = SUCCESS;
                        break;
                    }
                }
                else{
                    packet_ptr->result = FAIL;
                }
            }

            mysql_free_result(res);
            res = NULL;
            printf("Login Failed!\n");
            printf("%s %s %d\n", packet_ptr->message, packet_ptr->info.username, packet_ptr->result);

            if(write(clnt_socket, packet_ptr, sizeof(PACKET)) != readlen)
                perror("write() error!");
            }
    }
}

//login duplication check
int check_duplication(int clnt_sock, char *username){

    for(int i = 0; i < clnt_cnt; i++){
        if(strcmp(username, clnt_list[i].username) == 0){
            return 1;   //Duplication
        }
    }

    pthread_mutex_lock(&mutex);
    for(int i = 0; i < clnt_cnt; i++){
        if(clnt_sock == clnt_list[i].socket){
            strcpy(clnt_list[i].username, username);
        }
    }
    pthread_mutex_unlock(&mutex);
    
    return 0;
}

/*
typedef struct{
    int id;
	char title[MAXLEN];
	int accessible;
	int users;
    int port;
}Lobby;

typedef struct{
    int result;
    char message[MAXLEN];
    int status;
    user_info info;
    int lobby_idx;                  //0 ~ 19
    Lobby lobby_List[MAX_LOBBY];    //every lobby's information
    //Game game;
}PACKET;

Lobby lobby_list[MAX_LOBBY];
int lobby_client_sock[MAX_LOBBY][4];
int game_on_socket[MAX_LOBBY] = {0};
*/

int lobby(int clnt_socket, PACKET* packet_ptr){

    int readlen = 0;
    //int i;
    //int result = FAIL;
    packet_ptr->result = FAIL;

    printf("Client[%d] Lobby!!\n", clnt_socket);    //Debugging

    while(packet_ptr->result != SUCCESS){
        //printf("Wait for client input\n");

        if((readlen = read(clnt_socket, packet_ptr, sizeof(PACKET))) == -1)
            perror("read() error!");

        //Client Quit
        if((packet_ptr->result == QUIT) && (strcmp(packet_ptr->message, "QUIT") == 0)){
            //packet_ptr->lobby_List[packet_ptr->lobby_idx].users -= 1;
            //packet_ptr->lobby_idx = -1;
            client_close(clnt_socket);
            return QUIT;
        }
        /*
        if(packet_ptr->lobby_idx == -1){
            //send current lobby data

        }
        */
        //id: 0, title: 1, access: 2, users: 3
        for(int i = 0; i < MAX_LOBBY; i++){
            //printf("Check lobby,%s %d\n", lobby_list[i].title, lobby_list[i].id);
            if(packet_ptr->lobby_idx == i){
                if(lobby_list[i].accessible == 1){
                    if((++lobby_list[i].users) == 4)
                        lobby_list[i].accessible = 0;

                    //printf("%d %s %d %d %d\n", lobby_list[i].id, lobby_list[i].title, lobby_list[i].accessible, lobby_list[i].users, packet_ptr->lobby_List[i].port);
                    lobby_client_sock[i][(lobby_list[i].users - 1)] = clnt_socket;
                    //printf("%d %d %d %d \n", lobby_client_sock[i][0], lobby_client_sock[i][1], lobby_client_sock[i][2], lobby_client_sock[i][3]);
                    packet_ptr->result = SUCCESS;
                    //sleep(5);
                }
                else{
                    packet_ptr->result = FAIL;
                    strcpy(packet_ptr->message, "Not accessible!\n");
                    packet_ptr->lobby_List[i].accessible = lobby_list[i].accessible;
                }

            }
            
            packet_ptr->lobby_List[i].id = lobby_list[i].id;
            strcpy(packet_ptr->lobby_List[i].title, lobby_list[i].title);
            packet_ptr->lobby_List[i].accessible = lobby_list[i].accessible;
            packet_ptr->lobby_List[i].users = lobby_list[i].users;
            packet_ptr->lobby_List[i].port = lobby_list[i].port;

        }

        printf("%d %s %d %d %d\n", lobby_list[packet_ptr->lobby_idx].id, lobby_list[packet_ptr->lobby_idx].title, lobby_list[packet_ptr->lobby_idx].accessible, lobby_list[packet_ptr->lobby_idx].users, packet_ptr->lobby_List[packet_ptr->lobby_idx].port);
        printf("Result: %d\n", packet_ptr->result);
        if(write(clnt_socket, packet_ptr, sizeof(PACKET)) != readlen)
            perror("write() error!");

        if(packet_ptr->result == SUCCESS){
            printf("Client[%d] break\n", clnt_socket);
            break;
        }
    } 

    return SUCCESS;
}

int room(int clnt_socket, PACKET* packet_ptr){

    //send current room state
    int readlen = 0;

    printf("room!\n");
    printf("Id: %d\n", packet_ptr->lobby_idx + 1);

    //send client the current room data for screen
    //구현해야 함 - Packet 수정이 필요하여 논의 후 구현 아니면 message를 이용?

    packet_ptr->result = FAIL;

    while(packet_ptr->result != SUCCESS){

        for(int i = 0; i < MAX_LOBBY; i++){
            packet_ptr->lobby_List[i].id = lobby_list[i].id;
            strcpy(packet_ptr->lobby_List[i].title, lobby_list[i].title);
            packet_ptr->lobby_List[i].accessible = lobby_list[i].accessible;
            packet_ptr->lobby_List[i].users = lobby_list[i].users;
            packet_ptr->lobby_List[i].port = lobby_list[i].port;
        }

        //printf("Wait for client input\n");

        memset(packet_ptr->message, 0, MAXLEN);

        //if((readlen = read(clnt_socket, packet_ptr, sizeof(PACKET))) == -1)
        //    perror("read() error!");

        //Client Quit
        if((packet_ptr->result == QUIT) && (strcmp(packet_ptr->message, "QUIT") == 0)){
            lobby_list[packet_ptr->lobby_idx].users -= 1;
            client_close(clnt_socket);
            return QUIT;
        }

        /*
        if(strcmp(packet_ptr->message, "Ready") == 0){
            int i = 0;
            //화면 data 수정


            for(i = 0; i < MAX_LOBBY; i++){
                for(int j = 0; j < 4; j++){
                    if(lobby_client_sock[i][j] == clnt_socket){
                        ready[i][j] = YES;
                    }
                }
            }

            int t = 0;

            for(int j = 0; j < 4; j++){
                if(ready[i][j] == YES)
                    t += 1;
            }

            if(lobby_list[i].users > 1 && t == lobby_list[i].users){
                //Game on
                printf("All users Ready\n");
                lobby_list[i].accessible = 0;
                packet_ptr->result = SUCCESS;

            }
            else{
                packet_ptr->result = FAIL;
            }
        */

        if(lobby_list[packet_ptr->lobby_idx].accessible == 0){
            printf("Start game: %d %d %s %d %d %d\n", clnt_socket,lobby_list[packet_ptr->lobby_idx].id, lobby_list[packet_ptr->lobby_idx].title, lobby_list[packet_ptr->lobby_idx].accessible, lobby_list[packet_ptr->lobby_idx].users, packet_ptr->lobby_List[packet_ptr->lobby_idx].port);
            packet_ptr->result = SUCCESS;

            for(int i = 0; i < MAX_LOBBY; i++){
                packet_ptr->lobby_List[i].id = lobby_list[i].id;
                strcpy(packet_ptr->lobby_List[i].title, lobby_list[i].title);
                packet_ptr->lobby_List[i].accessible = lobby_list[i].accessible;
                packet_ptr->lobby_List[i].users = lobby_list[i].users;
                packet_ptr->lobby_List[i].port = lobby_list[i].port;
                //printf("%d, %d\n", packet_ptr->lobby_List[i].id , packet_ptr->lobby_List[i].users);
            }

            printf("%d game on - idx: %d\n", clnt_socket, packet_ptr->lobby_idx);
            write(clnt_socket, packet_ptr, sizeof(PACKET));
        }

        /*
        if(packet_ptr->lobby_List[packet_ptr->lobby_idx].users == 4){
            packet_ptr->result = SUCCESS;
        }
        */    
    }

    client_close(clnt_socket);
    return SUCCESS;

}

