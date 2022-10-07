#ifndef __PACKET__H__
#define __PACKET__H__

#include <openssl/md5.h>

#define SLEEP_SECONDS   10
#define PERIOD_SECONDS  0.010

typedef struct {
    struct {
        uint32_t    iNumber;
        int         iWords;
        uint8_t     Md5 [MD5_DIGEST_LENGTH];
    } Head;
    uint16_t    aBuffer[1600];
} _Packet;

#endif  //  #ifndef __PACKET__H__
