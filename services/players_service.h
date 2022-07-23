PlayerNode *root2 = NULL;
PlayerNode *prev2, *cur2;

PlayerNode *makeNewNode2(Player element)
{
    PlayerNode *new_player = (PlayerNode *)malloc(sizeof(PlayerNode));
    new_player->element = element;
    new_player->next = NULL;
    return new_player;
}

Player readNode2()
{
    Player tmp;
    printf("Input name:");
    scanf("%s", tmp.name);
    printf("Input password:");
    scanf("%s", tmp.password);
    printf("Input elo_rating:");
    scanf("%d", &tmp.elo_rating);

    return tmp;
}

void displayNode2(PlayerNode *p)
{
    if (p == NULL)
    {
        printf("Pointer problem\n");
        return;
    }
    Player tmp = p->element;
    // printf("%-20s\t%-17s\t%-25d%-25s%-25d%-p\n", tmp.name, tmp.password, tmp.elo_rating, tmp.ip,tmp.status, p->next);
    printf("%-20s\t%-17s\t%-25d%-25d%-p\n", tmp.name, tmp.password, tmp.elo_rating, tmp.status, p->next);
}

void insertAtHead2(Player ele)
{
    PlayerNode *new_player = makeNewNode2(ele);
    new_player->next = root2;
    root2 = new_player;
    cur2 = root2;
}

void insertAftercur2rent(Player element)
{
    PlayerNode *new_player = makeNewNode2(element);
    if (root2 == NULL)
    {
        root2 = new_player;
        cur2 = root2;
    }
    else if (cur2 == NULL)
    {
        printf("cur2rent pointer is nULL.\n");
        return;
    }
    else
    {
        new_player->next = cur2->next;
        cur2->next = new_player;
        prev2 = cur2;
        cur2 = cur2->next;
    }
}

void insertBeforecur2rent(Player e)
{
    PlayerNode *new_player = makeNewNode2(e);
    PlayerNode *tmp = root2;

    while (tmp != NULL && tmp->next != cur2 && cur2 != NULL)
    {
        tmp = tmp->next;
        prev2 = tmp;
    }

    if (root2 == NULL)
    {
        root2 = new_player;
        cur2 = root2;
        prev2 = NULL;
    }
    else
    {
        new_player->next = cur2;
        if (cur2 == root2)
        {
            root2 = new_player;
        }
        else
            prev2->next = new_player;
        cur2 = new_player;
    }
}

void freeList2(PlayerNode *root2)
{
    PlayerNode *to_free = root2;
    while (to_free != NULL)
    {
        root2 = root2->next;
        free(to_free);
        to_free = root2;
    }
}

void deletefirstElement2()
{
    PlayerNode *del = root2;
    if (del == NULL)
        return;
    root2 = del->next;
    free(del);
    cur2 = root2;
    prev2 = NULL;
}

void deletecur2()
{
    if (cur2 == NULL)
        return;
    if (cur2 == root2)
        deletefirstElement2();
    else
    {
        prev2->next = cur2->next;
        free(cur2);
        cur2 = prev2->next;
    }
}

PlayerNode *list_reverse2(PlayerNode *root2)
{
    PlayerNode *cur2, *prev2;
    cur2 = prev2 = NULL;
    while (root2 != NULL)
    {
        cur2 = root2;
        root2 = root2->next;
        cur2->next = prev2;
        prev2 = cur2;
    }
    return prev2;
}

void deleteAtposi2(int pos)
{
    cur2 = root2;
    int i;
    for (i = 0; i < pos; i++)
    {
        if (cur2->next != NULL)
        {
            prev2 = cur2;
            cur2 = cur2->next;
        }
        else
            break;
    }
    if (i < pos || pos < 0)
    {
        printf("No PlayerNode at postion.\n");
        return;
    }
    deletecur2();
}

void insertAtposi2(Player ele, int pos)
{
    int i;
    cur2 = root2;
    if (pos <= 0)
    {
        insertBeforecur2rent(ele);
        return;
    }

    for (i = 0; i < pos; i++)
    {
        if ((cur2->next) != NULL)
            cur2 = cur2->next;
        insertAftercur2rent(ele);
    }
}

void traversingList2(PlayerNode *root2)
{
    PlayerNode *p;
    for (p = root2; p != NULL; p = p->next)
        displayNode2(p);
}

Player updateUserInfo(char *name, int elo_rating)
{
    PlayerNode *p;

    for (p = root2; p != NULL; p = p->next)
    {
        if (strcmp(p->element.name, name) == 0)
        {
            p->element.elo_rating = elo_rating;
            traversingList2(root2);
            printf("Update success %s\n", p->element.name);
            return p->element;
        }
    }
}

void importTextFile(char *filename)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Failed to import from database!\n");
        return;
    }
    Player tmp;
    int i = 0;
    freeList2(root2);
    while (fscanf(fp, "%s %s %d %d", tmp.name, tmp.password, &tmp.elo_rating, &tmp.status) != EOF)
    {
        insertAtHead2(tmp);
        i++;
    }
    printf("Import %d record to linked list.\n", i);
    fclose(fp);
}

// Found the name at line
int getLine2(char *filename, char *name)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
    }
    Player tmp;
    int i = 0;
    int line = 0;
    int found = 0;
    while (fscanf(fp, "%s %s %d %d", tmp.name, tmp.password, &tmp.elo_rating, &tmp.status) != EOF)
    {
        line = line + 1;
        if (strcmp(name, tmp.name) == 0)
        {
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (found == 1)
    {
        printf("Found %s at line %d \n", name, line);
        return line;
    }
    else
        printf("Found at line 0\n");
    return 0;
}

// save name user withdata
void saveData1(Player ele)
{
    // update file path
    int line = getLine2("main_storage.txt", ele.name);
    FILE *fPtr;
    FILE *fTemp;
    char path[100];

    char buffer[1000];
    char newline[1000];
    int count;

    /* Remove extra new line character from stdin */
    fflush(stdin);

    printf("Replace '%d' line with: ", line);

    char tmp1[50];
    strcpy(newline, ele.name);
    strcat(newline, " ");
    strcat(newline, ele.password);
    strcat(newline, " ");
    sprintf(tmp1, "%d", ele.elo_rating);
    strcat(newline, tmp1);
    strcat(newline, " ");
    sprintf(tmp1, "%d", ele.status);
    strcat(newline, tmp1);
    strcat(newline, "\n");

    /*  Open all required files */
    fPtr = fopen("main_storage.txt", "r");
    fTemp = fopen("replace.tmp", "w");

    /* fopen() return NULL if unable to open file in given mode. */
    if (fPtr == NULL || fTemp == NULL)
    {
        /* Unable to open file hence exit */
        printf("\nUnable to open file.\n");
        printf("Please check whether file exists and you have read/write privilege.\n");
        exit(EXIT_SUCCESS);
    }

    /*
     * Read line from source file and write to destination
     * file after replacing given line.
     */
    count = 0;
    while ((fgets(buffer, 1000, fPtr)) != NULL)
    {
        count++;

        /* If current line is line to replace */
        if (count == line)
            fputs(newline, fTemp);
        else
            fputs(buffer, fTemp);
    }

    /* Close all files to release resource */
    fclose(fPtr);
    fclose(fTemp);

    /* Delete original source file */
    remove("main_storage.txt");

    /* Rename temporary file as original file */
    rename("replace.tmp", "main_storage.txt");

    printf("\nSuccessfully replaced '%d' line with '%s'.", line, newline);
}
void append(char *str)
{
    // append to last line
    FILE *fptr3;
    int i, n;

    char fname[20];

    printf("\n\n Append multiple lines at the end of a text file :\n");
    printf("------------------------------------------------------\n");
    printf(" Input the file name to be opened : ");

    // pthread_mutex_lock(&file_mutex);
    fptr3 = fopen("main_storage.txt", "a");
    printf(" Input the number of lines to be written : ");
    // scanf("%d", &n);
    printf(" The lines are : \n");
    fprintf(fptr3, "%s", str);
    fclose(fptr3);
    // pthread_mutex_unlock(&file_mutex);
}

void saveData(char *filename, Player user)
{
    FILE *fp;
    // pthread_mutex_lock(&file_mutex);
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
    }
    Player tmp;
    int i = 0;
    int line = 0;
    int found = 0;
    while (fscanf(fp, "%s %s %d", tmp.name, tmp.password, &tmp.elo_rating) != EOF)
    {
        line = line + 1;
        if (strcmp(user.name, tmp.name) == 0)
        {
            // fprintf(fp, "%s %s %d", tmp.name, tmp.password, tmp.elo_rating);
            fprintf(fp, "tuan %s %d", tmp.password, tmp.elo_rating);
            break;
        }
    }
    fclose(fp);
    // pthread_mutex_lock(&file_mutex);
}