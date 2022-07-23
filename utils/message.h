#include <stdio.h>
#include <pthread.h>

void print_err(char *msg)
{
    if (msg != NULL)
    {
        printf("[ ERROR ] %s !\n", msg);
        return;
    }
    printf("[ ERROR ] Something went wrong!\n");
}

void error(const char *msg)
{
    perror(msg);
    pthread_exit(NULL);
}

void print_msg(char *msg)
{
    if (msg != NULL)
    {
        printf("[ INFO ] %s !\n", msg);
        return;
    }
}