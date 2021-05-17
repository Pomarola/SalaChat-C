#ifndef CLIENT
#define CLIENT

#define MAX_LEN 1200
#define MSG_LEN 1024

void error(char *msg);

void *sendMsg();

void shut_down (int signal);

#endif