#ifndef _MY_3_H_
#define _MY_3_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include <ev.h>

typedef struct {
    // int iSocket;
    char *argv_1;
    char *argv_2;

    struct ev_loop* loop_1;
    ev_async        watcher_1;

    char *buffer;
    pthread_mutex_t LockBuffer;
} _ParamThread;

void *thread_3 (void* args);

#endif  //  #ifndef _MY_3_H_
