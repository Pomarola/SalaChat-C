#ifndef SERVER
#define SERVER

#include <pthread.h>

#define MAX_CLIENTS 15      // Maxima cantidad de cliente que soportará nuestro servidor
#define MAX_NICKNAME 150    // Maxima cantidad de caracteres para el nickname
#define MSG_LEN 1024        // Maxima cantidad de caracteres para mensajes de clientes
#define MAX_LEN 1200        // Maxima cantidad de caracteres para mensajes del servidor


typedef struct _client {
    int socket;
    char nickname[MAX_NICKNAME];
} client;

typedef struct _argsT {
    int sockClient;
    pthread_mutex_t mutex;
    client* clientArr;
} argsT_t;

// Busca el nickname en el array de clientes y si lo encuentra devuelve 1, caso contrario 0
int search_nickname(client* clientArray, char* nickname);

// Busca el nickname en el array de clientes y si lo encuentra devuelve el socket asociado, caso contrario -1
int obtain_socket(client* clientArray, char* nickname);

// Coloca un nuevo cliente en el array de clientes dado su nickname y su socket asociado
void new_client(client* clientArray, char* nickname, int socket);

// Busca a un cliente en el array de clientes dado su socket y reemplaza su nickname por uno nuevo
void change_nickname(client* clientArray, char* newNickname, int socket);

// Coloca el socket del cliente que se desea eliminar en -1 y el nickname en cadena vacia
void delete_client (client* clientArray, int sockClient);

// Devuelve un numero segun el tipo de string ingresado
int take_action (char* buf);

// Funcion para el hilo que atiende a cada cliente
void * worker(void* argsT);

void error(char *msg);

#endif