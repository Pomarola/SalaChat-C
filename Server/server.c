#include "server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int search_nickname(client* clientArray, char* nickname) {
  int found = 0, i = 0;

  for (; i < MAX_CLIENTS && !found; i++) {
    if (!strcmp(clientArray[i].nickname, nickname))
      found = 1;
  }

  return found;
}

int obtain_socket(client* clientArray, char* nickname) {
  int found = 0, i = 0, socket = -1;

  for (; i < MAX_CLIENTS && !found; i++) {
    if (!strcmp(clientArray[i].nickname, nickname))
      found = 1;
  }

  if (found) 
    socket = clientArray[i-1].socket;

  return socket;
}

void new_client(client* clientArray, char* nickname, int socket) {
  int i = 0;

  for (; clientArray[i].socket != -1; i++);

  strcpy(clientArray[i].nickname, nickname);
  clientArray[i].socket = socket;
}

void change_nickname(client* clientArray, char* newNickname, int socket) {
  int i = 0;

  for (; clientArray[i].socket != socket; i++);

  strcpy(clientArray[i].nickname, newNickname);
}

void delete_client (client* clientArray, int sockClient) {
  int found = 0, i = 0;

  for (; i < MAX_CLIENTS && !found; i++) {
    if (clientArray[i].socket == sockClient)
      found = 1;
  }

  if (i != MAX_CLIENTS) {
    clientArray[i-1].socket = -1;
    strcpy(clientArray[i-1].nickname,"");
  }
}

/* Devuelve:
0 msg al canal general
1 msg privado
2 cambio nickname
3 exit
4 comando no reconocido
*/
int take_action (char* buf) {
  int action = 0;

  if (buf[0] == '/') {
    if (!strncmp(buf, "/msg ", strlen("/msg ")))
      action = 1;
    else if (!strncmp(buf, "/nickname ", strlen("/nickname ")))
      action = 2;
    else if (!strcmp(buf, "/exit"))
      action = 3;
    else
      action = 4;
  }

  return action;
}

int main(int argc, char **argv){
  int sock, *soclient;
  struct sockaddr_in servidor, clientedir;
  socklen_t clientelen;
  pthread_t thread;
  pthread_attr_t attr;

  if (argc <= 1) error("Faltan argumentos");

  /* Creamos el socket */
  if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    error("Socket Init");

  /* Creamos a la dirección del servidor.*/
  servidor.sin_family = AF_INET; /* Internet */
  servidor.sin_addr.s_addr = INADDR_ANY; /**/
  servidor.sin_port = htons(atoi(argv[1]));

  /* Inicializamos el socket */
  if (bind(sock, (struct sockaddr *) &servidor, sizeof(servidor)))
    error("Error en el bind");

  printf("Binding successful, and listening on %s\n",argv[1]);

  /************************************************************/
  /* Creamos los atributos para los hilos.*/
  pthread_attr_init(&attr);
  /* Hilos que no van a ser *joinables* */
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  /************************************************************/

  /* Ya podemos aceptar conexiones */
  if(listen(sock, MAX_CLIENTS) == -1)
    error(" Listen error ");

  pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;

  client clientArray[MAX_CLIENTS];

  for (int i = 0; i < MAX_CLIENTS; i++) {
    clientArray[i].socket = -1;
  }

  for(;;){ /* Comenzamos con el bucle infinito*/
    /* Pedimos memoria para el socket */
    soclient = malloc(sizeof(int));

    /* Now we can accept connections as they come*/
    clientelen = sizeof(clientedir);
    if ((*soclient = accept(sock
                          , (struct sockaddr *) &clientedir
                          , &clientelen)) == -1)
      error("No se puedo aceptar la conexión. ");

    argsT_t argsT;

    argsT.mutex = mutexLock;
    argsT.sockClient = *soclient;
    argsT.clientArr = clientArray;
    /* Le enviamos el socket al hijo*/
    pthread_create(&thread , &attr , worker, (void *) &argsT);

    /* El servidor puede hacer alguna tarea más o simplemente volver a esperar*/
  }

  /* Código muerto */
  close(sock);

  return 0;
}

void * worker(void* argsT){
  pthread_mutex_t mutexLock = ((argsT_t*)argsT)->mutex;
  int soclient = ((argsT_t*)argsT)->sockClient;
  client* clientArray = ((argsT_t*)argsT)->clientArr;
  int working = 1, valid = 0, socketDest;
  char buf[MSG_LEN], userMsg[MSG_LEN], svMsg[MAX_LEN], nickname[MAX_NICKNAME], auxNickname[MAX_NICKNAME];

  send(soclient, "Ingrese un nickname", sizeof("Ingrese un nickname"), 0);
  while (!valid) {
    recv(soclient, buf, sizeof(buf), 0);
    stpcpy(nickname, buf);

    pthread_mutex_lock(&mutexLock);

    if (!search_nickname(clientArray, nickname)) {
      new_client(clientArray, nickname, soclient);
      pthread_mutex_unlock(&mutexLock);
      send(soclient, "Bienvenido, ya puede comenzar a chatear", sizeof("Bienvenido, ya puede comenzar a chatear"), 0);
      valid = 1;
    } else {
      pthread_mutex_unlock(&mutexLock);
      send(soclient, "El nickname esta en uso, intente con otro", sizeof("El nickname esta en uso, intente con otro"), 0);
    }
  }

  while (working) {
    recv(soclient, buf, sizeof(buf), 0);

    int action = take_action(buf);
    
    switch (action) {
    case 0:     // msg al canal general
      pthread_mutex_lock(&mutexLock);

      for (int i = 0; i < MAX_CLIENTS; i++){
        if (clientArray[i].socket != -1) {
          sprintf(svMsg, "[%s]: %s", nickname, buf);
          send(clientArray[i].socket, svMsg, sizeof(svMsg), 0);
        }
      }

      pthread_mutex_unlock(&mutexLock);
      break;

    case 1:   // msg privado
      strcpy(userMsg,"");
      sscanf(buf, "%*s %s %[^\n]", auxNickname, userMsg);
      pthread_mutex_lock(&mutexLock);

      socketDest = obtain_socket(clientArray, auxNickname);
      if (socketDest != -1){
        sprintf(svMsg, "[Privado] %s dice: %s", nickname, userMsg);
        send(socketDest, svMsg, sizeof(svMsg), 0);
      } else
        send(soclient, "No se reconoce al usuario con el que se esta intentando chatear", sizeof("No se reconoce al usuario con el que se esta intentando chatear"), 0);

      pthread_mutex_unlock(&mutexLock);
      break;

    case 2:     // cambio nickname
      sscanf(buf, "%*s %s", auxNickname);
      pthread_mutex_lock(&mutexLock);

      if (!search_nickname(clientArray, auxNickname)){
        change_nickname(clientArray, auxNickname, soclient);
        send(soclient, "Se cambio el nickname correctamente", sizeof("Se cambio el nickname correctamente"), 0);
        strcpy(nickname, auxNickname);
      } else  
        send(soclient, "Nickname ocupado, intente con otro", sizeof("Nickname ocupado, intente con otro"), 0);

      pthread_mutex_unlock(&mutexLock);
      break;

    case 3:     // exit
      send(soclient, "Hasta luego!", sizeof("Hasta luego!"), 0);

      pthread_mutex_lock(&mutexLock);
      delete_client(clientArray, soclient);
      pthread_mutex_unlock(&mutexLock);
      working = 0;
      break;
    
    default:      // comando no reconocido
      send(soclient, "Comando no reconocido", sizeof("Comando no reconocido"), 0);
      break;
    }
  }

  return NULL;
}

void error(char *msg){
  exit((perror(msg), 1));
}
