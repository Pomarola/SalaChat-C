#include "client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

int socketSv;
struct addrinfo *result;

void error(char *msg){
  exit((perror(msg), 1));
}

int main(int argc, char **argv){
  char buf[MAX_LEN];

  /*Chequeamos mínimamente que los argumentos fueron pasados*/
  if(argc != 3){
    fprintf(stderr,"El uso es \'%s IP port\'", argv[0]);
    exit(1);
  }

  /* Inicializamos el socket */
  if( (socketSv = socket(AF_INET , SOCK_STREAM, 0)) < 0 )
    error("No se pudo iniciar el socket");

  /* Buscamos la dirección del hostname:port */
  if (getaddrinfo(argv[1], argv[2], NULL, &result)){
    fprintf(stderr,"No se encontro el host: %s \n",argv[1]);
    exit(2);
  }

  if(connect(socketSv, (struct sockaddr *) result->ai_addr, result->ai_addrlen) != 0)
    /* if(connect(sock, (struct sockaddr *) &servidor, sizeof(servidor)) != 0) */
    error("No se pudo conectar :(. ");

  printf("La conexión fue un éxito!\n");

  signal(SIGINT, shut_down);

  pthread_t thread;
  int connected = 1;
  pthread_create(&thread, NULL, sendMsg, NULL);

  while(connected) {
    recv(socketSv, buf, sizeof(buf), 0);
    //buf[1199] = '\0';
    printf("%s\n", buf);
    if(!strcmp(buf, "Hasta luego!"))
      connected = 0;
  }
  /* Cerramos :D!*/
  freeaddrinfo(result);
  close(socketSv);

  return 0;
}

void * sendMsg() {
  char buf[MSG_LEN];

  while (1) {
    scanf(" %[^\n]", buf);
    if(!strcmp(buf, "/exit"))
      raise(SIGINT);
    send(socketSv, buf, sizeof(buf), 0);
  }

  return NULL;
}

void shut_down (int signal) {
  char buf[MAX_LEN];
  (void) signal;

  send(socketSv, "/exit", sizeof("/exit"), 0);
  recv(socketSv, buf, sizeof(buf), 0);
  printf("%s\n", buf);

  freeaddrinfo(result);
  close(socketSv);
  exit(0);
}