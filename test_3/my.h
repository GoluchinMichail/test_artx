#ifndef __MY__H__
#define __MY__H__

#include <stdint.h> 
#include <sys/un.h>

#define MAX_SIZE_PACKET 1024

void* thread_2 (void* args);

int GetUnixSock (char *pre_name, struct sockaddr_un *unServerAddress);
uint16_t _MyCrc (void *mem, int length);
uint16_t MyPsevdoCrc (uint32_t saddr, uint32_t daddr, uint8_t protocol, uint16_t lenCode);

typedef struct {
    int sock_in;                    //  eth0
    int sock_out;                   //  eth1

    struct sockaddr_un ServerEx;    //  адрес первого потока

    //  для проталкивания пакета до фиктивного места назначения
    int resend_Src_ip;
    int resend_Dest_ip;
} _Prm_Server;


typedef struct {

    _Prm_Server Share;

    int sock_client_ex; //  для обмена с первым потоком

    char Buffer_In  [MAX_SIZE_PACKET];  // буфер для чтения sock_in
    char Buffer_Ex  [MAX_SIZE_PACKET];  // буфер для чтения sock_client_ex

    // наблюдение за поступлениями пакетов (иногда, пока обрабатывается один пакет - поступает следующий)
    int boBusy;
    int iCountBusy;

} _Prm_Client;

#endif  //  #ifndef __MY__H__
