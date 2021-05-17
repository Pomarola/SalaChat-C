#ifndef SERVER
#define SERVER

#include <pthread.h>

/* Maxima cantidad de cliente que soportará nuestro servidor */
#define MSG_LEN 1024
#define MAX_CLIENTS 15
#define MAX_NICKNAME 150
#define MAX_LEN 1200

typedef struct _client {
    int socket;
    char nickname[MAX_NICKNAME];
} client;

typedef struct _argsT {
    int sockClient;
    pthread_mutex_t mutex;
    client* clientArr;
} argsT_t;


int search_nickname(client* clientArray, char* nickname);

int obtain_socket(client* clientArray, char* nickname);

void new_client(client* clientArray, char* nickname, int socket);

void change_nickname(client* clientArray, char* newNickname, int socket);

int delete_client (client* clientArray, int sockClient);

int take_action (char* buf);

void * worker(void* argsT);

void error(char *msg);

#endif