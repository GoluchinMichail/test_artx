
#include <semaphore.h>

typedef struct {
    int booStop;    //  сигнал об остановке
    int iSleep_ms;  //  задержка при осмотре буфера
    sem_t semaRing; //  факт пополнения буфера
    _Ring Ring;     //  буфер
} _Param_Processed;

void* Processed (void* args);
