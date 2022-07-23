#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <sys/types.h>
#include "utils/stdio_helper.h"
#include "config/constants.h"
#include "utils/message.h"

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
#include "auth_server/encryption.h"

// Symmetric 1 key for encrypt and decrypt
static _Atomic unsigned int cli_count = 0;
static int uid = 10;
static int roomUid = 1;
int firstElo, secondElo;

// client structure
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t auth_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "models/Room.h"

Client *clients[MAX_CONNECTION];
room_t *rooms[MAX_ROOM];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "utils/queue_handler.h"

int i = 0;

float Probability(int rating1, int rating2)
{
    return 1.0 * 1.0 / (1 + 1.0 * pow(10, 1.0 * (rating1 - rating2) / 400));
}

void EloRating(int Ra, int Rb, int K, int d)
{
    float Pb = Probability(Ra, Rb);

    float Pa = Probability(Rb, Ra);

    int a, b;
    if (d == 1)
    {
        firstElo = Ra + K * (1 - Pa);
        secondElo = Rb + K * (0 - Pb);
    }

    else if (d == 0)
    {
        firstElo = Ra + K * (0 - Pa);
        secondElo = Rb + K * (1 - Pb);
    }
    else
    {

        firstElo = Ra + K * (0.5 - Pa);
        secondElo = Rb + K * (0.5 - Pb);
    }

    /*fflush(stdout);
    printf( "Ra = %d Rb = %d", Ra,Rb );*/
}

void deliver(char *message, int uid)
{

    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CONNECTION; i++)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {

                if (write(clients[i]->sockfd, message, strlen(message)) < 0)
                {
                    printf("ERROR: write to descriptor failed\n");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

#include "auth_server/auth_handler.h"
#include "server_game/server_game.h"
void *handle_client(void *arg)
{
    char buffer[BUFF_SIZE];
    char command[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    int number;
    char fen_str[FEN_STR_LEN];
    char name[USERNAME_LEN];
    int leave_flag = 0;
    int flag = 0;
    int isRank = 0;
    int isLogin = 0; /* logout then isLogin=0 */
    Client *cli = (Client *)arg;
    char user[100];
    char password[100];

    // name nhan tin hieu
    recv(cli->sockfd, name, USERNAME_LEN, 0);

    cli->player_info.elo_rating = 1200;
    cli->player_info.status = 0;
    strcpy(cli->player_info.name, "Someone");

    sprintf(buffer, "> %s has joined\n", cli->player_info.name);
    printf("%s", buffer);

    bzero(buffer, BUFF_SIZE);
    strcpy(buffer, "SELECT_MODE|");
    strcat(buffer, "ok1");
    deliver(buffer, cli->uid);

    bzero(buffer, BUFF_SIZE);
    char *p;
    while (leave_flag == 0)
    {
        command[0] = '\x00';
        number = 0;

        int receive = recv(cli->sockfd, buffer, BUFF_SIZE, 0);

        if (receive > 0)
        {
            if (strlen(buffer) > 0)
            {

                // deliver(buffer, cli->uid);
                trim_lf(buffer, strlen(buffer));
                printf("> client: '%s' has been send '%s' command\n", cli->player_info.name, buffer);
                sscanf(buffer, "%[^|]|%i", &command[0], &number);

                if (strstr(buffer, "GUEST"))
                {
                    handleGuest(name, cli, buffer);
                }
                else if (strstr(buffer, "SIGNUP"))
                { // TODO:luu vao file
                    handleReg(cli, buffer);
                }
                else if (strstr(buffer, "LOGOUT"))
                {
                    handleLogOut(&isLogin, cli, buffer);
                }
                else if (strstr(buffer, "LOGIN"))
                {
                    handleLogin(&isLogin, cli, buffer);
                }

                else if (strcmp(buffer, "CREATE") == 0 || strcmp(buffer, "CREATE RANK") == 0)
                {
                    handleCreateRoom(&isLogin, &flag, cli, buffer);
                }

                // random
                else if (strcmp(command, "JOIN") == 0)
                {
                    handleJoin(&isLogin, &number, cli);
                }
                else if (strcmp(command, "LIST") == 0)
                {
                    handleListRooms(cli);
                }
                else if (strcmp(command, "LEAVE") == 0)
                {
                    handleLeave(cli);
                }
                else if (strcmp(command, "START") == 0)
                {
                    handleStart(cli);
                }
                else if (strcmp(command, "PLAY") == 0)
                {
                    sscanf(buffer, "%[^|]|%s", &command[0], fen_str);
                    handlePlay(fen_str, cli);
                }
                else
                {
                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, "INVALID_CMD|");
                    strcat(buffer, "Invalid command\n");
                    deliver(buffer, cli->uid);
                }
            }
        }
        else if (receive == 0 || strcmp(buffer, "exit") == 0)
        {

            // TODO: THEM PHAN LOGOUT VAO DAY
            if (isLogin == 1)
            {

                isLogin = 0;
                PlayerNode *n;
                for (n = root2; n != NULL; n = n->next)
                {
                    if (strcmp(cli->player_info.name, n->element.name) == 0)
                    {
                        n->element.status = 0;

                        saveData1(n->element);
                        // TODO : luu data vaofile khi logout
                    }
                }
                traversingList2(root2);
            }
            sprintf(buffer, "%s has left\n", cli->player_info.name);
            printf("%s", buffer);
            leave_flag = 1;
        }
        else
        {
            print_err("Something went wrong");
            leave_flag = 1;
        }

        bzero(buffer, BUFF_SIZE);
    }

    bzero(buffer, BUFF_SIZE);
    strcpy(buffer, "EXIT|");
    strcat(buffer, "bye");
    deliver(buffer, cli->uid);

    close(cli->sockfd);
    queue_remove_client(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Syntax error!\n-->Usage : ./server [PORT_NUMBER] (i.e. ./server 8888) !\n");
        exit(EXIT_FAILURE);
    }
    // int sockfd = setup_server(atoi(argv[1]));
    int sockfd;
    struct sockaddr_in serv_addr;

    // validate input port number
    int const PORT = atoi(argv[1]);
    if (PORT == 0)
    {
        print_err("Invalid port number specified");
        exit(EXIT_FAILURE);
    }

    // socket construction
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        print_err("Failed to construct a listener socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Signals
    signal(SIGPIPE, SIG_IGN);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        print_err("Failed to bind the listener socket");
    }

    int option = 1;
    int connection_fd = 0;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    // start listening
    if (listen(sockfd, MAX_CONNECTION) < 0)
    {
        print_err("Failed to listen");
        return EXIT_FAILURE;
    }

    flashScreen();

    printf("----------------------------------------------\n");
    printf("-- Xiangqi playground running on port: %d --\n", PORT);
    printf("----------------------------------------------\n\n");
    importTextFile("main_storage.txt");
    traversingList2(root2);
    while (1)
    {
        socklen_t clilen = sizeof(cli_addr);
        connection_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        // check dor max clients
        if ((cli_count + 1) == MAX_CONNECTION)
        {
            printf("Maximun of clients are connected, Connection rejected");
            close(connection_fd);
            continue;
        }
        // printf("New connection:%d\n", connection_fd);
        //  clients settings
        Client *cli = (Client *)malloc(sizeof(Client));
        cli->address = cli_addr;
        cli->sockfd = connection_fd;
        cli->uid = uid++;
        printf("Uid:%d\n", cli->uid);
        // add client to queue
        queue_add_client(cli);
        pthread_create(&tid, NULL, &handle_client, (void *)cli);

        // reduce CPU usage
        sleep(1);
    }

    return EXIT_SUCCESS;
}
