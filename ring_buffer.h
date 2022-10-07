#ifndef __RING__H__
#define __RING__H__

#include <pthread.h>
#include "packet.h"

typedef struct {
    pthread_mutex_t lock_RW;    //  блокировка чтения/записи
    _Packet Packet;
} _Record_Ring;

typedef struct {
    int iReader,  iWriter;
    _Record_Ring  Buffer [16];
//    _Record_Ring  Buffer   [338]; // OK %/
} _Ring;

#define ACOUNT(a) sizeof(a)/sizeof(a[0])

int InitRingBuffer (_Ring *rec);
void WriteToRingBuffer (_Ring *ring, _Packet *packet);
int ReadFromRingBuffer (_Ring *ring, _Packet *packet);  // error= 0
void CloseRingBuffer (_Ring *rec);

#endif  //  #ifndef __RING__H__ ...