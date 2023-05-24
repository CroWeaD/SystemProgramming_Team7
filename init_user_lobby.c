#include <stdio.h>
#include <mysql.h>  //<> | ""   //locate mysql.h -> /opt/homebrew/Cellar/mysql/8.0.31/include/mysql/mysql.h
//gcc mysql.c -o mysql $(mysql_config --libs --cflags)

//essential
char *server = "localhost";  //server path
char *user = "root";    //user name
char *password = "1q2w3e4r@@"; //password of the user

//optional
char *database = "BlueMarble";  //DataBase name
//char *userTable = "user_tb";    //table name
/*
database BlueMarble

create table user_tb(
    useridx int primary key auto_increment,
    userid varchar(300) unique not null,
    userpw varchar(300) not null,
    username varchar(300) not null,
    userphone varchar(300) not null,
    useremail varchar(300),
    regdate datetime default now()
    );

    create table lobby(
        id int primary key auto_increment,
        title varchar(300) not null,
        access int,
        users int
    );

*/

MYSQL *conn;    //variable for mysql connection  
MYSQL_RES *res;  //variable for result of query  
MYSQL_ROW row;   //real data which result from query


void connect(){

    char *query = "show tables";    //query

    //1. initialize
    //mysql_init(NULL) allocates or initializes a MYSQL structure suitable for mysql_real_connect
    if((conn = mysql_init(NULL)) == NULL)
        perror("mysql_init() error!");

    //2. real connection
    //mysql_real_connect(MYSQL *conn, char *server, char *user, char *password, char *database, 0, NULL, 0)
    if(mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL){
        mysql_close(conn);
        perror("mysql_real_connect() error!");
    }

    //3. query
    if(mysql_query(conn, query)){
        mysql_close(conn);
        perror("mysql_query() error!");
    }
    else{
        res = mysql_use_result(conn);   //MYSQL *mysql_use_result(MYSQL *conn);    -> after query, use this fuction to get result
        printf("%s\n", query);
        printf("length: %d\n", mysql_num_fields(res));  //number of fields

        while((row = mysql_fetch_row(res)))   //MYSQL_ROW *mysql_fetch_row(MYSQL_RES *res)    -> get a row from the result
            printf("%s\n", row[0]);
    }   


    //init lobby - only use once!!
    char temp[300];
    char room_title[100];
    for(int i = 1; i <= 20; i++){

        sprintf(room_title, "Room %d", i);
        sprintf(temp, "INSERT INTO lobby (title, access, users) VALUES('%s', %d, %d)", room_title, 1, 0);
        printf("%s\n", temp);
        query = temp;

        if(mysql_query(conn, query)){
            mysql_free_result(res);     //free result
            mysql_close(conn);
            perror("mysql_query() error!");
            exit(1);
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
    printf("mysql.c!\n");
    connect();
}