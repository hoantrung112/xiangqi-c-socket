#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../config/constants.h"
typedef struct Player
{
    char name[USERNAME_LEN];
    char password[PASSWORD_LEN];
    int elo_rating;
    int status;
} Player;

typedef struct t
{
    Player element;
    struct t *next;
} PlayerNode;

#include "../services/players_service.h"

