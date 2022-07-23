#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <sys/types.h>

#include "config/constants.h"
#include "utils/message.h"
#include "utils/stdio_helper.h"
#include "auth_server/encryption.h"

#include "client_game/client_game.h"
#include "client_auth/client_authen.h"

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[USERNAME_LEN]; // playerName
int player = 1;

pthread_t lobby_thread;     // send message thread
pthread_t recv_msg_thread;  // recv msg thread
pthread_t multiplayer_game; // game thread

char ip[1000];
int port;
void startScreen();
void catch_ctrl_c_and_exit()
{
    flag = 1;
}

char winState[100] = "";    /*state game:win or lose*/
char updatedElo[100] = "";  /*elo_player_1*/
char updatedElo1[100] = ""; /*elo_player_2*/

char gameType[100]; /*game mode: normal or rank*/

// dua tren turn declare
void split(char a[100], int playerTurn)
{
    // printf("Str:%s\n",a);
    bzero(gameType, 100);
    bzero(winState, 100);

    bzero(updatedElo, 100);
    bzero(updatedElo1, 100);

    char *token = strtok(a, "|");

    strcpy(gameType, token);

    // if rank
    if (strcmp(gameType, "[RANK]") == 0)
    {
        token = strtok(NULL, "|");

        strcat(updatedElo, token);
        token = strtok(NULL, "|");
        strcat(updatedElo1, token);
    }

    token = strtok(NULL, "|");
    strcat(winState, token);
}
void handle_write_file(char *fen_str)
{
    FILE *fp = fopen("database/data.txt", "w");
    if (fp == NULL)
    {
        printf("Cannot access database to write! Exiting...\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "%s", fen_str);
    fclose(fp);
}
void showBoard(char fen_str[FEN_STR_LEN])
{
    printf("Current FEN string is: %s\n", fen_str);
}

void initBoard(char fen_str[])
{
    strcpy(fen_str, INIT_FEN_STR);
}

void menu();

void *lobby(void *arg);

void recv_msg_handler();

void *multiplayerGame(void *arg)
{
    int playerTurn;
    playerTurn = player; // ngchoi 1 hay 2
    char namePlayer1[32];
    char namePlayer2[32];

    strcpy(namePlayer1, name);
    char fen_str[FEN_STR_LEN];
    int round = 1;
    int gameStatus = 1;
    int valid_play = 0;
    int played;

    char *nameCurrentPlayer;  /*name player 1*/
    char *nameCurrentPlayer1; /*name opponent*/

    // message
    char message[BUFF_SIZE] = {};
    char rep[BUFF_SIZE] = {};
    char status[100];

    // TODO : FORMAT   PLAYER2| NAME_PLAYER_2
    int receive = recv(sockfd, rep, BUFF_SIZE, 0);

    if (receive > 0)
    {
        // handle message
        strcpy(status, strtok(rep, "|"));
        strcpy(message, strtok(NULL, "|"));
        setbuf(stdin, 0);
        trim_lf(message, strlen(message));
        sscanf(message, "%s", &namePlayer2[0]);

        setbuf(stdout, 0);
        setbuf(stdin, 0);

        bzero(message, BUFF_SIZE);
        bzero(rep, 100);
        initBoard(fen_str);
        round = 1;

        while (round < 10 && gameStatus == 1)
        {
            if (playerTurn == 1)
            {
                nameCurrentPlayer = (char *)&namePlayer1;
                nameCurrentPlayer1 = (char *)&namePlayer2;
            }
            else
            {
                nameCurrentPlayer1 = (char *)&namePlayer1;
                nameCurrentPlayer = (char *)&namePlayer2;
            }

            showBoard(fen_str);

            printf("\nRound: %d", round);
            printf("\nPlayer: %s\n", nameCurrentPlayer);

            while (valid_play == 0)
            {
                bzero(message, BUFF_SIZE);
                bzero(rep, 100);

                int receive = recv(sockfd, rep, BUFF_SIZE, 0);
                // strcpy(message, message);
                printf("Receive:%s\n", rep);
                if (receive > 0)
                {
                    valid_play = 1;
                    strcpy(status, strtok(rep, "|"));
                    strcpy(message, strtok(NULL, "|"));
                    setbuf(stdin, 0);
                    setbuf(stdout, 0);
                    // your turn
                    if (strcmp(message, "1") == 0)
                    {
                        int is_valid_fen_str = 0;
                        do
                        { // Turn
                            printf("Enter your FEN string: ");
                            scanf("%s", fen_str);

                            // TODO: (Hieu) checking user input here
                            is_valid_fen_str = 1;

                        } while (!is_valid_fen_str);
                        if (valid_play == 1)
                        {
                            makeMove(fen_str, message);
                            send(sockfd, message, strlen(message), 0);
                            bzero(message, BUFF_SIZE);
                        }
                    }
                    else if (strcmp(message, "2") == 0)
                    {
                        printf("The other player is playing...\n");

                        played = 0;

                        while (played == 0)
                        { // TODO: NHAN MOVE

                            int receive = recv(sockfd, message, BUFF_SIZE, 0);
                            // strcpy(message, message);

                            if (receive > 0)
                            {
                                sscanf(message, "%s", fen_str);
                                // wrtie to file for browser rendering
                                handle_write_file(fen_str);
                                played = 1;
                            }
                        }

                        valid_play = 1;
                    }
                }
                else
                {
                    valid_play = 0;
                }
            }

            if (playerTurn == 1)
            {
                playerTurn = 2;
            }
            else
            {
                playerTurn = 1;
            }
            if (strcmp(fen_str, "GAME_OVER") == 0)
            {
                gameStatus = 0;
            }

            round++;
            valid_play = 0;
            bzero(message, BUFF_SIZE);
        }

        bzero(message, BUFF_SIZE);
        // TODO : FORMAT | message
        int receive = recv(sockfd, message, BUFF_SIZE, 0);
        // strcpy(message, message);
        if (receive > 0)
        {
            setbuf(stdin, 0);
            setbuf(stdout, 0);

            showBoard(fen_str);
            split(message, playerTurn);
            // printf("\nPlayer '%s' win!\nUpdated elo_rating: '%s'", winState,updatedElo);
            if (strcmp(gameType, "[NORMAL]") == 0)
            {
                if (strcmp(winState, "win") == 0)
                {
                    printf("\n You win\n");
                    if (playerTurn == 1)
                    {
                        // nameCurrentPlayer = (char *)&namePlayer1;
                        // nameCurrentPlayer1 = (char *)&namePlayer2;
                        printf("\nPlayer '%s' lose!\n", nameCurrentPlayer);
                    }
                    else
                    {
                        printf("\nPlayer '%s' lose!\n", nameCurrentPlayer1);
                    }
                }
                else if (strcmp(winState, "lose") == 0)
                {
                    printf("\nYou lose");
                    if (playerTurn == 1)
                    {
                        // nameCurrentPlayer = (char *)&namePlayer1;
                        // nameCurrentPlayer1 = (char *)&namePlayer2;
                        printf("\nPlayer '%s' win!\n", nameCurrentPlayer);
                    }
                    else
                    {
                        printf("\n Player %s  win!", nameCurrentPlayer1);
                    }
                }
                else
                {
                    printf("\nDraw match");
                }
            }

            else
            {
                if (strcmp(winState, "win") == 0)
                {
                    printf("\nYou win!\nUpdated elo_rating: '%s'", updatedElo); // khi tk1 win
                    printf("\nPlayer '%s' lose!\nUpdated elo_rating: '%s'", nameCurrentPlayer1, updatedElo1);
                }
                else if (strcmp(winState, "lose") == 0)
                {

                    printf("\nYou lose!\nUpdated elo_rating: '%s'", updatedElo1);
                    printf("\nPlayer '%s' win!\nUpdated elo_rating: '%s'", nameCurrentPlayer, updatedElo);
                }
                else
                {
                    printf("\nDraw match");

                    if (playerTurn == 1)
                    {
                        printf("\nYour  elo_rating: '%s'", updatedElo);
                        printf("\nPlayer '%s'  elo_rating: '%s'", nameCurrentPlayer, updatedElo1);
                    }
                    else
                    {
                        printf("\nYour  elo_rating: '%s'", updatedElo1);
                        printf("\nPlayer '%s'  elo_rating: '%s'", nameCurrentPlayer1, updatedElo);
                    }
                }
            }
            printf("\nEnd of the game!\n");

            sleep(6);

            if (pthread_create(&lobby_thread, NULL, &lobby, NULL) != 0)
            {
                printf("ERROR: pthread\n");
                return NULL;
            }

            if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
            {
                printf("ERROR: pthread\n");
                return NULL;
            }

            pthread_detach(pthread_self());   // thu hoi tai nguyen
            pthread_cancel(multiplayer_game); // huy thread ngay
        }
    }

    return NULL;
}
char username[100]; // hold name temp when not login
void *lobby(void *arg)
{
    char buffer[BUFF_SIZE] = {};
    char uname[100], password[100];
    while (1)
    {
        str_overwrite_stdout();
        fgets(buffer, BUFF_SIZE, stdin);
        for (int i = 0; buffer[i] != '\0'; i++)
        {
            if (buffer[i] >= 'a' && buffer[i] <= 'z')
            {
                buffer[i] = buffer[i] - 32;
            }
        }
        trim_lf(buffer, BUFF_SIZE);

        if (strcmp(buffer, "GUEST") == 0)
        {
            memset(uname, '\0', (strlen(uname) + 1));
            guest(uname, buffer);
            strcpy(name, uname);
        }

        if (strcmp(buffer, "JOIN") == 0)
        {
            joinRoom(buffer);
        }
        if (strcmp(buffer, "SIGNUP") == 0)
        {
            memset(uname, '\0', (strlen(uname) + 1));
            memset(password, '\0', (strlen(password) + 1));
            signUp(uname, password, buffer);
        }
        if (strcmp(buffer, "LOGIN") == 0)
        {

            login(uname, password, buffer);
            strcpy(name, uname);
        }
        if (strcmp(buffer, "LOGOUT") == 0)
        {

            memset(buffer, '\0', (strlen(buffer) + 1));
            logOut(name, buffer);

            // str_overwrite_stdout();
        }
        if (strcmp(buffer, "EXIT") == 0)
        {
            break;
        }
        else
        {
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(buffer, BUFF_SIZE);
    }

    catch_ctrl_c_and_exit();

    return NULL;
}

void menu()
{
    printf("[COMMANDS]:\n");
    printf("\t <list>\t\t\t  List all Xiangqi rooms\n");
    printf("\t <create>\t\t   Casual Room\n");
    printf("\t <create rank>\t\t  Ranked room\n");
    printf("\t <join>\t\t  Join in a Xiangqi room\n");
    printf("\t <leave>\t\t\t  Leave Xiangqi room\n");
    printf("\t <start>\t\t\t  Start a Xiangqi game\n");
    printf("\t <login>\t\t\t  Log in \n");
    printf("\t <signup>\t\t  Dont' have an account? Register\n");
    printf("\t <exit>\t\t\t  Close terminal\n\n");
}
void logged_menu()
{
    printf("[COMMANDS]:\n");
    printf("\t <list>\t\t\t  List all Xiangqi rooms\n");
    printf("\t <create>\t\t   Normal Room\n");
    printf("\t <create rank>\t\t  Ranked room\n");
    printf("\t <join>\t\t  Join in a Xiangqi room\n");
    printf("\t <leave>\t\t\t  Leave Xiangqi room\n");
    printf("\t <start>\t\t\t  Start a Xiangqi game\n");
    printf("\t <logout>\t\t  Log out\n\n");
}
void selectMode()
{

    printf("<guest>  -login as guest\n");
    printf("<login>  -signin\n");
    printf("<signup> -register new account\n\n");
}
void recv_msg_handler()
{
    char message[BUFF_SIZE] = {};
    char rep[BUFF_SIZE] = {};
    char status[100];
    flashScreen();

    while (1)
    {

        int receive = recv(sockfd, rep, BUFF_SIZE, 0);
        strcpy(status, strtok(rep, "|"));
        strcpy(message, strtok(NULL, "|"));
        if (receive > 0)
        {
            if (strcmp(status, "SELECT_MODE") == 0)
            { 
                selectMode();
                str_overwrite_stdout();
            }
            else if (strcmp(status, "MENU") == 0 || strcmp(status, "GAME_OVER") == 0)
            { 

                menu();
                str_overwrite_stdout();
            }
            else if (strcmp(status, "START_1") == 0)
            {
                pthread_cancel(lobby_thread);
                player = 1;
                if (pthread_create(&multiplayer_game, NULL, (void *)multiplayerGame, NULL) != 0)
                {
                    printf("ERROR: pthread\n");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(pthread_self());
                pthread_cancel(recv_msg_thread);
            }
            else if (strcmp(status, "START_2") == 0)
            {
                pthread_cancel(lobby_thread);
                player = 2;
                if (pthread_create(&multiplayer_game, NULL, (void *)multiplayerGame, NULL) != 0)
                {
                    printf("ERROR: pthread\n");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(pthread_self());
                pthread_cancel(recv_msg_thread);
            }
            else if (strcmp(status, "LGIN_SUCC") == 0) 
            {

                printf("Logged in successfully!\n");
                strcpy(username, name);
                strcpy(name, message);
                flashScreen();
                printf("Hello:%s\n", name);
                logged_menu();
                str_overwrite_stdout();
            }
            else if (strcmp(status, "LOGOUT_SUCC") == 0)
            {
                printf(">Until next time %s\n", username);
                strcpy(username, "");
                strcpy(name, username);
                sleep(1);
                flashScreen();
                menu();
                str_overwrite_stdout();
            }
            else if (strcmp(status, "ROOM_LISTS") == 0)
            {
                printf("%s", message);
                str_overwrite_stdout();
            }
            else if (receive == 0)
            {
                break;
            }
            else
            {
                printf("[%s]-[SERVER] %s", status, message);
                str_overwrite_stdout();
            }

            bzero(message, BUFF_SIZE);
            bzero(rep, BUFF_SIZE);
            bzero(status, 100);
        }
    }
}

int conectGame(char *ip, int port)
{
    setbuf(stdin, 0);
    strcpy(name, "CONNECTED");
    struct sockaddr_in server_addr;

    // socket settings
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // connect to the server
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1)
    {
        print_err("Failed to connect");
        return EXIT_FAILURE;
    }

    // send the name
    send(sockfd, name, USERNAME_LEN, 0);

    if (pthread_create(&lobby_thread, NULL, &lobby, NULL) != 0)
    {
        print_err("Pthread error");
        return EXIT_FAILURE;
    }

    if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
    {
        print_err("Pthread error");
        return EXIT_FAILURE;
    }

    while (1)
    {
        if (flag)
        {
            printf("\nUntil next time!\n");
            break;
        }
    }

    close(sockfd);

    return 0;
}

void startScreen()
{
    int option = 0;

    flashScreen();

    signal(SIGINT, catch_ctrl_c_and_exit);

    while (option < 1 || option > 3)
    {
        printf("Welcome to Xiangqi playground");
        printf("\n1 - Play locally");
        printf("\n2 - Play online");
        printf("\n3 - About Xiangqi");
        printf("\n4 - Exit");
        printf("\nMake your choice and hit ENTER: ");

        scanf("%d", &option);

        switch (option)
        {
        case 1:
            flashScreen();
            option = 0;
            printf("Open the pinned HTML file on browser to enjoy a local game!\n");
            break;
        case 2:
            flashScreen();
            option = 0;
            exit(conectGame(ip, port));
            break;
        case 3:
            flashScreen();
            option = 0;
            printf("How about googling? =))))\n");
            break;
        case 4:
            flashScreen();
            option = 0;
            printf("Until next time, dear!\n");
            exit(0);
            break;
        default:
            flashScreen();
            option = 0;
            printf("Invalid option!\n");
            continue;
            break;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("error, too many or too few arguments\n");
        printf("Correct format is ./client <ip> <port>");
        return 1;
    }
    //
    port = atoi(argv[2]);
    strcpy(ip, argv[1]);
    // printf("%s %d", argv[1], port);
    startScreen();

    return 0;
}
