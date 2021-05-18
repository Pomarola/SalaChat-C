#ifndef CLIENT
#define CLIENT

#define MAX_LEN 1200
#define MSG_LEN 1024


void error(char *msg);

// Funcion para el hilo que envia los mensajes del cliente
void *sendMsg();

// Funcion para desconectarse del servidor (/exit o ctrl+c)
void shut_down (int signal);

#endif