#include <stdio.h>
#include <mysql.h>      //gcc option $(mysql_config --libs --cflags)
//gcc server.c -o server -I/opt/homebrew/Cellar/mysql/8.0.33/include/mysql -L/opt/homebrew/Cellar/mysql/8.0.33/lib -lmysqlclient -L/opt/homebrew/opt/openssl@1.1/lib -lssl -lcrypto -lresolv
/*
    mysql 8.0.33 version has -lzlib -lzstd linking problem
*/
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "packet.h"

#define MAX_CLNT MAX_LOBBY * 4

int clnt_cnt = 0;

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
//char *userTable = "user_tb";

MYSQL *conn = NULL;    //variable for mysql connection  

//mutex
pthread_mutex_t mutex;

//socket
int serv_sock, clnt_sock;

//Packet
Lobby lobby_list[MAX_LOBBY];
int game_on_socket[MAX_LOBBY] = {0};

//Functions
void sigint_handler(int signal, siginfo_t *siginfo, void *context);
void client_close(int clnt_socket);

void *clnt_main(void* clnt_socket);

int login(int clnt_socket, PACKET* packet_ptr);
int check_duplication(int clnt_sock, char *username);

int lobby(int clnt_socket, PACKET* packet_ptr);
int room(int clnt_socket, PACKET* packet_ptr);
void game(int clnt_socket, PACKET* packet_ptr);
void init_server();

MYSQL_RES * mysql_do_query(char *query);

/* Main function */
int main(int argc, char *argv[]){
    //socket
    struct sockaddr_in clnt_adr;
    int clnt_adr_sz;    //socklen_t == int
    //mutex
    pthread_t t_id;

    init_server();

    //Debug Message
    printf("Server On!!\n");

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

    //Just in case - results error
    //mysql_free_result(res);     //free result
    //mysql_close(conn);    //close connection
    //close(serv_sock);  
    return 0;
}

void init_server(){
    //socket
    struct sockaddr_in serv_adr;
    //signal
    struct sigaction handler;
    sigset_t blocked;

    MYSQL_RES *res = NULL;  //variable for result of query  //change to local variable  
    MYSQL_ROW row;   //real data which result from query

    system("clear");

    //0. mysql
    if((conn = mysql_init(NULL)) == NULL)
        perror("mysql_init() error!");
    
    if(mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL){
        mysql_close(conn);
        perror("mysql_real_connect() error!");
    }

    //1. mutex
    pthread_mutex_init(&mutex, NULL);  

    //2. signal handling
    handler.sa_sigaction = sigint_handler;  
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
    serv_adr.sin_port = htons(SERVER_PORT);

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1){
        perror("bind() error!");
        exit(1);
    }
        
    if(listen(serv_sock, 5) == -1){
        perror("listen() error!");
        exit(1);
    }
    //printf("Init\n");

    //Init lobby data
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
    //res = NULL;

    //init client list
    for(int i = 0; i < MAX_CLNT; i++){
        clnt_list[i].pid = 0;
        clnt_list[i].socket = 0;
        strcpy(clnt_list[i].username,"");
    }

    return;
}

//Closing Server
void sigint_handler(int signal, siginfo_t *siginfo, void *context){
    system("clear");

    printf("Turnning off the server...\n");

    //close every socket that is connected to server
    for(int i = (clnt_cnt - 1); i >= 0; i--){
        client_close(clnt_list[i].socket);
    }

    /*
    if(res != NULL)
        mysql_free_result(res);     //free result
    */
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

MYSQL_RES * mysql_do_query(char *query){

    //printf("%s\n", query);
    if(mysql_query(conn, query)){
        mysql_close(conn);
        perror("mysql_query() error!");
    }

    //res = mysql_use_result(conn);   //MYSQL *mysql_use_result(MYSQL *conn);    -> after query, use this fuction to get result
    //int fields = mysql_num_fields(res);

    return mysql_use_result(conn);    //use    mysql_free_result(res);     //free result after mysql_do_query();
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
        while(1){
            //2. Lobby
            result = lobby(clnt_sock, &packet);
            if(result == QUIT){
                client_close(clnt_sock);
                return NULL;
            }
            else if(result == SUCCESS){
                result = room(clnt_sock, &packet);
            }

            
            if(result == SUCCESS){
                //room(clnt_sock, packet.lobby.id);    //debugging
                //3. Game
                printf("Game start!\n");
                game(clnt_sock, &packet);
            }   
        }
         
    }
    
    
    if(packet.result == QUIT){
        client_close(clnt_sock);
    }

    return NULL;
}

int login(int clnt_socket, PACKET* packet_ptr){

    MYSQL_RES *res = NULL;  //variable for result of query  
    MYSQL_ROW row;   //real data which result from query
    char *query;    //query
    
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

        //printf("ID: %s, PASSWORD: %s\n", packet_ptr->info.id, packet_ptr->info.password); //Debugging
        //check
        //id: 1, password: 2, username: 3
        res = mysql_do_query(query);
            
        while((row = mysql_fetch_row(res))){   //MYSQL_ROW *mysql_fetch_row(MYSQL_RES *res)    -> get a row from the result
            printf("%s %s %s\n", row[1], row[2], row[3]);
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

int lobby(int clnt_socket, PACKET* packet_ptr){

    int readlen = 0;
    int i;
    //int result = FAIL;
    packet_ptr->result = FAIL;

    printf("Client[%d] Lobby!!\n", clnt_socket);    //Debugging

    while(packet_ptr->result != SUCCESS){
        printf("Wait for client input\n");

        if((readlen = read(clnt_socket, packet_ptr, sizeof(PACKET))) == -1)
            perror("read() error!");

        //Client Quit
        if((packet_ptr->result == QUIT) && (strcmp(packet_ptr->message, "QUIT") == 0)){
            client_close(clnt_socket);
            return QUIT;
        }
        /*
        if(packet_ptr->lobby_idx == -1){
            //send current lobby data

        }
        */
        //id: 0, title: 1, access: 2, users: 3
        for(i = 0; i < MAX_LOBBY; i++){
            //printf("Check lobby,%s %d\n", lobby_list[i].title, lobby_list[i].id);
            if(packet_ptr->lobby_idx == i){
                if(lobby_list[i].accessible == 1){
                    if((++lobby_list[i].users) == 4)
                        lobby_list[i].accessible = 0;

                    printf("%d %s %d %d %d\n", lobby_list[i].id, lobby_list[i].title, lobby_list[i].accessible, lobby_list[i].users, packet_ptr->lobby_List[i].port);
                    packet_ptr->result = SUCCESS;
                }
                else{
                    packet_ptr->result = FAIL;
                    strcpy(packet_ptr->message, "Full!\n");
                    packet_ptr->lobby_List[i].accessible = lobby_list[i].accessible;
                }

            }
            
            packet_ptr->lobby_List[i].id = lobby_list[i].id;
            strcpy(packet_ptr->lobby_List[i].title, lobby_list[i].title);
            packet_ptr->lobby_List[i].accessible = lobby_list[i].accessible;
            packet_ptr->lobby_List[i].users = lobby_list[i].users;
            packet_ptr->lobby_List[i].port = lobby_list[i].port;

        }

        if(write(clnt_socket, packet_ptr, sizeof(PACKET)) != readlen)
            perror("write() error!");
        /*
        for(i = 0; i < MAX_LOBBY; i++){
            if(strcmp(packet_ptr->lobby_List[i].title, lobby_list[i].title) == 0 || packet_ptr->lobby.id == lobby_list[i].id){
                if(lobby_list[i].accessible == 1){

                    printf("Lobby OK\n");
                    
                    packet_ptr->lobby.port = lobby_list[i].port;

                    if((++lobby_list[i].users) == 4)
                        lobby_list[i].accessible = 0;

                    packet_ptr->lobby.users = lobby_list[i].users;
                    packet_ptr->lobby.accessible = lobby_list[i].accessible;

                    if(write(clnt_socket, packet_ptr, sizeof(PACKET)) != readlen)
                        perror("write() error!");

                    printf("%d %s %d %d %d\n", lobby_list[i].id, lobby_list[i].title, lobby_list[i].accessible, lobby_list[i].users, packet_ptr->lobby.port);
                    
                    //Add room code 

                    return SUCCESS;

                }
                else{
                    strcpy(packet_ptr->message, "Full!\n");
                    packet_ptr->lobby.accessible = lobby_list[i].accessible;
                }
            }
        }*/
        /*
        if(i == MAX_LOBBY){
            strcpy(packet_ptr->lobby.title, "Wrong Input\n");
        }

        if(write(clnt_socket,packet_ptr, sizeof(PACKET)) != readlen)
            perror("write() error!");
        */
        
    } 

    return SUCCESS;
}

int room(int clnt_socket, PACKET* packet_ptr){

    //send current room state

    printf("room!\n");
    printf("Id: %d\n", packet_ptr->lobby_idx + 1);


    while(packet_ptr->result != SUCCESS){

        if(packet_ptr->lobby_List[packet_ptr->lobby_idx].users == 4){
            packet_ptr->result = SUCCESS;
        }

        write(clnt_socket, &packet, sizeof(PACKET));
        
    }

    return SUCCESS;

}

void game(int clnt_socket, PACKET* packet_ptr){

    int new_serv_sock;
    struct sockaddr_in new_serv_adr, clnt_adr;
    int clnt_adr_sz = sizeof(clnt_adr);
    int option, optlen;

    pid_t pid;

    printf("Game!\n");
    printf("Id: \n");

    pthread_mutex_lock(&mutex);
    if(game_on_socket[packet_ptr->lobby_idx] == 0){
        pid = fork();
        game_on_socket[packet_ptr->lobby_idx] = 1;
    }
    pthread_mutex_unlock(&mutex);

    if(pid == 0){

        printf("New Socket!! %d\n",lobby_list[packet_ptr->lobby_idx].port );
        //game
        new_serv_sock = socket(PF_INET, SOCK_STREAM, 0);

        optlen = sizeof(option);
        option = 1;
        setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, optlen);   //set SO_REUSEADDR

        memset(&new_serv_adr, 0, sizeof(new_serv_adr));
        new_serv_adr.sin_family = AF_INET;
        new_serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        new_serv_adr.sin_port = htons(lobby_list[packet_ptr->lobby_idx].port);

        if(bind(new_serv_sock, (struct sockaddr*) &new_serv_adr, sizeof(new_serv_adr)) == -1){
            perror("bind() error!");
            exit(1);
        }
            
        if(listen(new_serv_sock, 5) == -1){
            perror("listen() error!");
            exit(1);
        }
        
        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, (socklen_t*) &clnt_adr_sz);

        printf("New server!!\n");
    }
    else{
        printf("New Socket parent\n");
    }  

    pthread_mutex_lock(&mutex);

    if(game_on_socket[packet_ptr->lobby_idx] == 1){
        prinf("Game end!\n"); 
        game_on_socket[packet_ptr->lobby_idx] = 0;
    }

    pthread_mutex_unlock(&mutex);
}
