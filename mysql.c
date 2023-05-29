#include <stdio.h>
#include <mysql.h>  
#include <errno.h>
#include <stdlib.h>
//<> | ""   //locate mysql.h -> /opt/homebrew/Cellar/mysql/8.0.31/include/mysql/mysql.h
//gcc mysql.c -o mysql $(mysql_config --libs --cflags)

//essential
char *server = "localhost";  //server path
char *user = "root";    //user name#include </usr/include/mysql/mysql.h>
char *password = "0000"; //password of the user

//optional
char *database = "BlueMarble";  //DataBase name
char *userTable = "user_tb";    //table name

//Variables
static MYSQL *conn;    //variable for mysql connection  
static MYSQL_RES *res;  //variable for result of query  
static MYSQL_ROW row;   //real data which result from query

char user_names[5][300] = {
	"Aerial",
	"Cordelia",
	"Macbeth",
	"Hamlet",
	"Othello",
};

void my_connect(){

    char *query;// = "show tables";    //query
    char temp[1300];
    char userid[300];
    char userpw[300];
    char username[300];
    char userphone[300];
    char room_title[100];
    char input;
    
    system("clear");
	
    //1. initialize
    //mysql_init(NULL) allocates or initializes a MYSQL structure suitable for mysql_real_connect
    printf("*****************mysql_init()*****************\n");
    if((conn = mysql_init(NULL)) == NULL){
        perror("mysql_init() error!");
        exit(1);
    }
        
	
    printf("\n");
    
    //2. real connection
    //mysql_real_connect(MYSQL *conn, char *server, char *user, char *password, char *database, 0, NULL, 0)
    printf("*************mysql_real_connect()*************\n");
    if(mysql_real_connect(conn, server, user, password, NULL, 0, NULL, 0) == NULL){
        mysql_close(conn);
        perror("mysql_real_connect() error!");
        exit(1);
    }

    printf("\n************CREATE Database & Table************\n");

    query = "CREATE DATABASE BlueMarble";
    
    if(mysql_query(conn, query)){
        //mysql_close(conn);
        if(errno == 0){
            printf("● Database BlueMarble is already created\n");
            
            printf("\nDrop Database BlueMarble? [y|Y for yes, others for no]: ");
            input = getchar();
            getchar();

            if(input == 'y' || input == 'Y'){
                query = "DROP DATABASE BlueMarble";
                if(mysql_query(conn, query)){
                    //mysql_close(conn);
                    if(errno != 0)
                        perror("mysql_query() error!");
                }
                printf("mysql> DROP DATABASE BlueMarble;\n");

                query = "CREATE DATABASE BlueMarble";
                if(mysql_query(conn, query)){
                    //mysql_close(conn);
                    if(errno != 0)
                        perror("mysql_query() error!");
                }
                printf("mysql> CREATE DATABASE BlueMarble;\n");
            }
            else{
                printf("Pass\n");
            }
            
        }
        else
            perror("● mysql_query() error!");
    }

    printf("\n");
    
    query = "USE BlueMarble";
    
    if(mysql_query(conn, query)){
        //mysql_close(conn);
        perror("mysql_query() error!");
    }
    
    query = "CREATE TABLE user_tb(useridx int primary key auto_increment, userid varchar(300) unique not null, userpw varchar(300) not null, username varchar(300) not null, userphone varchar(300), useremail varchar(300), regdate datetime default now());";
    
    if(mysql_query(conn, query)){
        //mysql_close(conn);
        if(errno == 0){
        	printf("● Table user_tb is already created\n");

            printf("\nDrop Table user_tb? [y|Y for yes, others for no]: ");
            input = getchar();
            getchar();

            if(input == 'y' || input == 'Y'){
                query = "DROP TABLE user_tb";
                if(mysql_query(conn, query)){
                    //mysql_close(conn);
                    if(errno != 0)
                        perror("mysql_query() error!");
                }
                printf("mysql> DROP TABLE user_tb;\n");

                query = "CREATE TABLE user_tb(useridx int primary key auto_increment, userid varchar(300) unique not null, userpw varchar(300) not null, username varchar(300) not null, userphone varchar(300) not null, useremail varchar(300), regdate datetime default now());";
                if(mysql_query(conn, query)){
                    //mysql_close(conn);
                    if(errno != 0)
                        perror("mysql_query() error!");
                }
                printf("mysql> CREATE TABLE user_tb;\n");
            }
            else{
                printf("Pass\n");
            }
        }
        else
            perror("● mysql_query() error!");
    }

    printf("\n");
    
    query = "CREATE TABLE lobby(id int primary key auto_increment, title varchar(300) unique not null, access int, users int);";
    
    if(mysql_query(conn, query)){
        //mysql_close(conn);
        if(errno == 0){
        	printf("● Table lobby is already created\n");

            printf("\nDrop Table lobby? [y|Y for yes, others for no]: ");
            input = getchar();
            getchar();

            if(input == 'y' || input == 'Y'){
                query = "DROP TABLE lobby";
                if(mysql_query(conn, query)){
                    //mysql_close(conn);
                    if(errno != 0)
                        perror("mysql_query() error!");
                }
                printf("mysql> DROP TABLE lobby;\n");

                query = "CREATE TABLE lobby(id int primary key auto_increment, title varchar(300) unique not null, access int, users int);";
                if(mysql_query(conn, query)){
                    //mysql_close(conn);
                    if(errno != 0)
                        perror("mysql_query() error!");
                }
                printf("mysql> CREATE TABLE lobby;\n");
            }
            else{
                printf("Pass\n");
            }
        }
        else
        	perror("● mysql_query() error!");
    }
    
    printf("\n");
    
    //init user _tb - only use once!!
    printf("*****************user_tb init*****************\n");
    
    for(int i = 1; i <= 5; i++){
        sprintf(userid, "test%d", i);
        sprintf(userpw, "%d%d%d%d", i, i, i, i);
        //sprintf(username, "%s", userid);
        sprintf(userphone, "010-%d%d%d%d-%d%d%d%d", i, i, i, i, i, i, i, i);
        sprintf(temp, "INSERT INTO user_tb (userid, userpw, username, userphone) VALUES('%s', '%s', '%s', '%s')", userid, userpw, user_names[i - 1], userphone);
        //printf("%s\n", temp);
        query = temp;

        if(mysql_query(conn, query)){
            //mysql_free_result(res);     //free result
            //mysql_close(conn);
            if(errno == 0){
        		printf("● Table user_tb is already initialized\n");
        	}
        	else
        		perror("mysql_query() error!");
            break;
            //exit(1);
        }
        else{
            res = mysql_use_result(conn);   //MYSQL *mysql_use_result(MYSQL *conn);    -> after query, use this fuction to get result
            printf("%s\n", query);
        }  
    }

	printf("\n");
    
    //init lobby - only use once!!
    printf("******************lobby init******************\n");
    for(int i = 1; i <= 20; i++){

        sprintf(room_title, "Room %d", i);
        sprintf(temp, "INSERT INTO lobby (title, access, users) VALUES('%s', %d, %d)", room_title, 1, 0);
        //printf("%s\n", temp);
        query = temp;

        if(mysql_query(conn, query)){
            //mysql_free_result(res);     //free result
            //mysql_close(conn);
            if(errno == 0){
        		printf("● Table lobby is already initialized\n");
        	}
        	else
        		perror("mysql_query() error!");
            break;
            //exit(1);
        }
        else{
            res = mysql_use_result(conn);   //MYSQL *mysql_use_result(MYSQL *conn);    -> after query, use this fuction to get result
            printf("%s\n", query);
        }  
    }
    mysql_free_result(res);     //free result
    mysql_close(conn);    //close connection
    exit(0);
}

int main(){
    my_connect();
}
