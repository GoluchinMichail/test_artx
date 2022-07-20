#ifndef _MY_2_H_
#define _MY_2_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>

#include <ev.h>

typedef struct {
    int iSocket;
    struct ev_loop* loop_1;
    ev_async        watcher_1;

    char acBuffer [1024];
    pthread_mutex_t LockBuffer;
} _ParamThread;

void* thread_2 (void* args);

#endif  //  #ifndef _MY_2_H_
