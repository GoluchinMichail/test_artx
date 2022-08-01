#ifndef _MY_2_H_
#define _MY_2_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <ev.h>

typedef struct {
    int iExchange;  //  для связи потоков
    struct sockaddr_un unServerAddress;

    int iSock;      //  клиент (телнет/прочее)
} _Prm;

void* thread_2 (void* args);
int GetUnixSock (char *pre_name, struct sockaddr_un *unServerAddress);

#endif  //  #ifndef _MY_2_H_
