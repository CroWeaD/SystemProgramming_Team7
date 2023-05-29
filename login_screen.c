#include <ncursesw/curses.h>
#include <wchar.h>
#include <string.h>
#include "packet.h"
#include <locale.h>

#define LEFT 'a'
#define RIGHT 'd'
#define UP 'w'
#define DOWN 's'
#define SPACEBAR ' '
#define BLANK "                                                  "


#define WIDTH 50
#define HEIGHT 15
#define LEFT_BUTTON_X 9
#define RIGHT_LEFT_BUTTON_Y 10
#define RIGHT_BUTTON_X 32

#define COLOR_PAIR_DEAFULT 1
#define COLOR_PAIR_DEFAULT_REVERSE 2

static enum State_login{BACK=0, INIT=0, SIGNIN, SIGNUP, CONTINUE};

static PACKET packet;
static char username[20] = "";

void startHighlight(WINDOW* win, int choice, char* str1, int x1, int y1, char* str2, int x2, int y2, int c1, int c2);
int start_screen(WINDOW* win);
int signIn(WINDOW* win, int serv_sock);
int signUp(WINDOW* win, int serv_sock);

char* login2(int* arg) {
    //int *arg;
    
    setlocale(LC_CTYPE,"");
    int serv_sock = *arg;
    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    enum State_login state = INIT;
    int flag = 1;

    // 색상 설정
    start_color();
    init_pair(COLOR_PAIR_DEAFULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_PAIR_DEFAULT_REVERSE, COLOR_BLACK, COLOR_WHITE);

    // start screen
    WINDOW *win = newwin(HEIGHT, WIDTH, 3, 6);
    wbkgd(win, COLOR_PAIR(COLOR_PAIR_DEFAULT_REVERSE));

    wrefresh(win);
    while(flag) {
        switch(state) {
            case INIT:
                state = start_screen(win);
                break;
            case SIGNIN:
                state = signIn(win, serv_sock);
                break;
            case SIGNUP:
                state = signUp(win, serv_sock);
                break;
            case CONTINUE:
                flag = 0;
        }
    }

    // 종료
    delwin(win);
    endwin();
    return username;
}

void startHighlight(WINDOW* win, int choice, char* str1, int x1, int y1, char* str2, int x2, int y2, int c1, int c2) {
    if(choice==c1) {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DEAFULT));
        mvwprintw(win, x1, y1, str1);
        wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT_REVERSE));
        mvwprintw(win, x2, y2, str2);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT_REVERSE));
    } else if(choice==c2) {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT_REVERSE));
        mvwprintw(win, x1, y1, str1);
        wattron(win, COLOR_PAIR(COLOR_PAIR_DEAFULT));
        mvwprintw(win, x2, y2, str2);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DEAFULT));
    }
    wrefresh(win);
}

int start_screen(WINDOW* win) {
    int choice = SIGNIN;

    mvwprintw(win, 2, 13, "Welcome to Blue Marble!");
    mvwprintw(win, RIGHT_LEFT_BUTTON_Y, LEFT_BUTTON_X, "[Sign in]");
    mvwprintw(win, RIGHT_LEFT_BUTTON_Y, RIGHT_BUTTON_X, "[Sign up]");

    while(1) {       
        fflush(stdin);
        startHighlight(win, choice, "[Sign in]", RIGHT_LEFT_BUTTON_Y, LEFT_BUTTON_X, 
                                    "[Sign up]", RIGHT_LEFT_BUTTON_Y, RIGHT_BUTTON_X, SIGNIN, SIGNUP);
        int ch = getchar();
        if(ch==LEFT) {
            choice = SIGNIN;
        } else if(ch==RIGHT) {
            choice = SIGNUP;
        } else if(ch==SPACEBAR) {
            wclear(win);
            break;
        }
    }
    return choice;
}

int signIn(WINDOW* win, int serv_sock) {
    int choice = CONTINUE;
    int readlen = 0;

    mvwprintw(win, 2, 11, "Enter your ID and password!");
    mvwprintw(win, 6, 2, "ID: ");
    mvwprintw(win, 8, 2, "PW: ");
    mvwprintw(win, RIGHT_LEFT_BUTTON_Y, LEFT_BUTTON_X, "[Sign in]");
    mvwprintw(win, RIGHT_LEFT_BUTTON_Y, RIGHT_BUTTON_X, "[Back]");
    wrefresh(win);

    // 커서 이동
    wmove(win, 6, 6);

    // 입력 받기
    char id[20];
    char pw[20];
    echo();
    curs_set(1);
    wgetstr(win,id);
    wmove(win, 8, 6);
    wrefresh(win);
    wgetstr(win,pw);
    curs_set(0);
    noecho();
    
    strcpy(packet.info.id, id);
    strcpy(packet.info.password, pw);
    strcpy(packet.info.username, "LOGIN");
    packet.result = 0;

    while(1) {    
        fflush(stdin);
        startHighlight(win, choice, "[Sign in]", RIGHT_LEFT_BUTTON_Y, LEFT_BUTTON_X, 
                                    "[Back]", RIGHT_LEFT_BUTTON_Y, RIGHT_BUTTON_X, CONTINUE, BACK);
        int ch = getchar();
        if(ch==LEFT) {
            choice = CONTINUE;
        } else if(ch==RIGHT) {
            choice = BACK;
        } else if(ch==SPACEBAR) {
            if(choice == CONTINUE) {
                
                write(serv_sock, &packet, sizeof(PACKET));

                if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
                    perror("read() error!");  

                if(packet.result == SUCCESS){
                    strcpy(username, packet.info.username);
                } else {
                    mvwprintw(win, 2, 11, BLANK);
                    mvwprintw(win, 2, 11, packet.message);
                    wrefresh(win);
                    sleep(1);
                    choice = SIGNIN;
                }
            }
            // 로그인 정보 확인
            wclear(win);
            break;
        }
    }
    return choice;
}

int signUp(WINDOW* win, int serv_sock) {
    int choice = SIGNIN;
    int readlen = 0;

    mvwprintw(win, 2, 13, "Welcome to Blue Marble!");
    mvwprintw(win, 3, 12, "Let's begin the adventure");
    mvwprintw(win, 6, 2, "NAME: ");
    mvwprintw(win, 7, 2, "ID: ");
    mvwprintw(win, 8, 2, "PW: ");
    mvwprintw(win, RIGHT_LEFT_BUTTON_Y, LEFT_BUTTON_X, "[Sign up]");
    mvwprintw(win, RIGHT_LEFT_BUTTON_Y, RIGHT_BUTTON_X, "[Back]");
    wrefresh(win);

    // 커서 이동
    wmove(win, 6, 8);

    // 입력 받기
    char name[20];
    char id[20];
    char pw[20];
    echo();
    curs_set(1);
    wgetstr(win, name);
    wmove(win, 7, 6);
    wgetstr(win, id);
    wmove(win, 8, 6);
    wrefresh(win);
    wgetstr(win, pw);
    curs_set(0);
    noecho();

    strcpy(packet.info.id, id);
    strcpy(packet.info.password, pw);
    strcpy(packet.info.username, name);
    packet.result = 0;

    while(1) {       
        fflush(stdin);
        startHighlight(win, choice, "[Sign up]", RIGHT_LEFT_BUTTON_Y, LEFT_BUTTON_X, 
                                    "[Back]", RIGHT_LEFT_BUTTON_Y, RIGHT_BUTTON_X, SIGNIN, BACK);
        int ch = getchar();
        if(ch==LEFT) {
            choice = SIGNIN;
        } else if(ch==RIGHT) {
            choice = BACK;
        } else if(ch==SPACEBAR) {
            if(choice==SIGNIN) {
                write(serv_sock, &packet, sizeof(PACKET));

                if((readlen = read(serv_sock, &packet, sizeof(PACKET))) == -1)
                    perror("read() error!");  

                if(packet.result == SUCCESS){
                    
                } else {
                    mvwprintw(win, 2, 11, BLANK);
                    mvwprintw(win, 2, 11, packet.message);
                    wrefresh(win);
                    sleep(1);
                    choice = SIGNUP;
                }
            }
            // 아이디 중복 확인?
            wclear(win);
            break;
        }
    }
    return choice;
}