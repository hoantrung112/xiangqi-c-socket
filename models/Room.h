#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Players.h"
// add mutex when delete create join login
typedef struct
{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    Player player_info;
    char username[100];

} Client;
// name khi chua login tmp
typedef struct
{
    char fen_str[FEN_STR_LEN];
    int gameStatus;
    int round;
    int playerTurn;
} game_t;

// room structure
typedef struct
{
    Client *player1;
    Client *player2;
    unsigned int uid;
    char state[100];
    game_t *game;
    char roomType[10];
} room_t;

struct node_t
{
    room_t element;
    struct node_t *next;
};
/*rewrite read node diplay node*/
typedef struct node_t node;
node *root = NULL;
node *prev, *cur;

node *makeNewNode(room_t element)
{
    node *new = (node *)malloc(sizeof(node));
    new->element = element;
    new->next = NULL;
    return new;
}

void displayNode(node *p)
{
    if (p == NULL)
    {
        printf("Pointer problem\n");
        return;
    }
    room_t tmp = p->element;
    printf("Game room:%d\n", tmp.uid);
    /*
    printf("Round:%d\nPlayer Turn:%d", tmp.game->round, tmp.game->playerTurn);
    printf("Room\n");
    printf("Roomid:%d\nRoom type:%-20s\nState:%-100s\n", tmp.uid, tmp.roomType, tmp.state);
    printf("Player\n");*/
    printf("User 1:%s\nElo:%d\n", tmp.player1->player_info.name, tmp.player1->player_info.elo_rating);
    // printf("User 2:%s\nElo:%d", tmp.player2->player_info.name, tmp.player2->player_info.elo_rating);
}

void insertAtHead(room_t ele)
{
    pthread_mutex_lock(&rooms_mutex);
    node *new = makeNewNode(ele);
    new->next = root;
    root = new;
    cur = root;
    pthread_mutex_unlock(&rooms_mutex);
}

void insertAfterCurrent(room_t element)
{
    node *new = makeNewNode(element);
    if (root == NULL)
    {
        root = new;
        cur = root;
    }
    else if (cur == NULL)
    {
        printf("Current pointer is nULL.\n");
        return;
    }
    else
    {
        new->next = cur->next;
        cur->next = new;
        prev = cur;
        cur = cur->next;
    }
}

void insertBeforeCurrent(room_t e)
{
    node *new = makeNewNode(e);
    node *tmp = root;

    while (tmp != NULL && tmp->next != cur && cur != NULL)
    {
        tmp = tmp->next;
        prev = tmp;
    }

    if (root == NULL)
    {
        root = new;
        cur = root;
        prev = NULL;
    }
    else
    {
        new->next = cur;
        if (cur == root)
        {
            root = new;
        }
        else
            prev->next = new;
        cur = new;
    }
}

void freeList(node *root)
{
    node *to_free = root;
    while (to_free != NULL)
    {
        root = root->next;
        free(to_free);
        to_free = root;
    }
}

void deletefirspassement()
{
    node *del = root;
    if (del == NULL)
        return;
    root = del->next;
    free(del);
    cur = root;
    prev = NULL;
}

void deletecur()
{
    if (cur == NULL)
        return;
    if (cur == root)
        deletefirspassement();
    else
    {
        prev->next = cur->next;
        free(cur);
        cur = prev->next;
    }
}

node *list_reverse(node *root)
{
    node *cur, *prev;
    cur = prev = NULL;
    while (root != NULL)
    {
        cur = root;
        root = root->next;
        cur->next = prev;
        prev = cur;
    }
    return prev;
}

void deleteAtposi(int roomNumber)
{
    printf("ROOM:%d\n", roomNumber);
    int pos = -1;
    node *p = root;
    int index = 0;
    int found = 0;
    pthread_mutex_lock(&rooms_mutex);

    // traverse till then end of the linked list
    while (p != NULL)
    {
        pos++;
        if (p->element.uid == roomNumber)
        {
            found = 1;
            break;
        }
        p = p->next;
    }
    /*
    for(p=root;p!=NULL;p++){
        pos = pos + 1;
        if(p->element.uid == roomNumber){
            found = 1;
            break;
        }
    }*/
    if (found == 0)
    {
        pos = -1;
    }
    cur = root;
    int i;
    for (i = 0; i < pos; i++)
    {
        if (cur->next != NULL)
        {
            prev = cur;
            cur = cur->next;
        }
        else
            break;
    }
    if (i < pos || pos < 0)
    {
        printf("No node at postion.\n");
        return;
    }
    deletecur();

    pthread_mutex_unlock(&rooms_mutex);
}

void insertAtposi(room_t ele, int pos)
{
    int i;
    cur = root;
    if (pos <= 0)
    {
        insertBeforeCurrent(ele);
        return;
    }

    for (i = 0; i < pos; i++)
    {
        if ((cur->next) != NULL)
            cur = cur->next;
        insertAfterCurrent(ele);
    }
}

void traversingList(node *root)
{
    node *p;
    for (p = root; p != NULL; p = p->next)
        displayNode(p);
}