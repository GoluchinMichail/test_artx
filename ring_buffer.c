
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include "ring_buffer.h"

int InitRingBuffer (_Ring *rec) {
    int iResult = 0;    //  set OK
    rec->iReader = 0;
    rec->iWriter = 0;
    for (int i=0;i<ACOUNT(rec->Buffer);i++)
        iResult |= pthread_mutex_init (&rec->Buffer[i].lock_RW, NULL);
    return iResult;
}

void CloseRingBuffer (_Ring *rec) {
    for (int i=0;i<ACOUNT(rec->Buffer);i++)
        pthread_mutex_destroy (&rec->Buffer[i].lock_RW);
}

void WriteToRingBuffer (_Ring *ring, _Packet *packet) {
    // пишем без оглядки на "читателя"
    _Record_Ring *rec = &ring->Buffer[ring->iWriter];

    pthread_mutex_lock (&rec->lock_RW);
    memcpy (&rec->Packet,packet,sizeof(*packet));
    pthread_mutex_unlock (&rec->lock_RW);

    ring->iWriter++;
    if (ring->iWriter >= ACOUNT(ring->Buffer))
        ring->iWriter = 0;
}

int ReadFromRingBuffer (_Ring *ring, _Packet *packet) {
    int result = 0; //  set ERROR
    // лишь бы не обогнать "писателя"
    if (ring->iReader != ring->iWriter) {
        _Record_Ring *rec = &ring->Buffer[ring->iReader];

        pthread_mutex_lock (&rec->lock_RW);
        memcpy (packet,&rec->Packet,sizeof(*packet));
        pthread_mutex_unlock (&rec->lock_RW);
        result = ring->iReader +1; // set OK

        ring->iReader++;
        if (ring->iReader >= ACOUNT(ring->Buffer))
            ring->iReader = 0;
    }
    return result;
}
