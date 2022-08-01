#include <stdio.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/un.h>
#include <time.h>
#include <error.h>

int GetUnixSock (char *pre_name, struct sockaddr_un *unServerAddress) {
    int sockfd;

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) >= 0) {
        struct sockaddr_un un_addr;
        memset (&un_addr, 0, sizeof(un_addr));
        un_addr.sun_family = AF_UNIX;

        // оригинальное имя файла сопровождения %/
        snprintf (un_addr.sun_path, sizeof(un_addr.sun_path)-1, "%s__%u", pre_name, clock());
        unlink (un_addr.sun_path);

        int caddrlen =
            sizeof(un_addr.sun_family) + strlen(un_addr.sun_path);
        if (bind(sockfd, (struct sockaddr*)&un_addr, sizeof(un_addr)) < 0) {
            perror ("-ERROR  bind()");
            close (sockfd);
            sockfd = -1;
        } else if (unServerAddress)
            memcpy (unServerAddress,&un_addr,sizeof(*unServerAddress));
        
    } else
        perror ("-ERROR  socket()");

    return sockfd;
}

uint16_t _MyCrc (void *mem, int length) {
    uint32_t Crc = 0;
    uint8_t *bytes = (uint8_t *) mem;
    for (int i=0;i<length;i++) {
        if (i & 1)
            Crc += ((uint16_t)bytes[i]) << 8; // как старший байт слова
        else
            Crc += bytes[i];
        Crc = (Crc & 0xFFFF) + (Crc >> 16); // учесть переполнения
    }
    return Crc;
}

uint16_t MyPsevdoCrc (uint32_t saddr, uint32_t daddr, uint8_t protocol, uint16_t lenCode) {
    uint32_t dPsev;

    #pragma pack(push,1)
    struct {
        uint32_t saddr;
        uint32_t daddr;
        uint8_t Zero;
        uint8_t protocol;
        uint16_t lenCode;
    } Q;
    #pragma pack(pop)

    Q.saddr = saddr;
    Q.daddr = daddr;
    Q.Zero = 0;
    Q.protocol = protocol;
    Q.lenCode = lenCode;
    dPsev = _MyCrc(&Q,sizeof(Q));

/*  другой вариант

    dPsev = htons(saddr);
    dPsev += htons(saddr>>16);
    dPsev = (dPsev & 0xFFFF) + (dPsev >> 16); // учесть переполнения

    dPsev += htons(daddr);
    dPsev = (dPsev & 0xFFFF) + (dPsev >> 16);
    dPsev += htons(daddr>>16);
    dPsev = (dPsev & 0xFFFF) + (dPsev >> 16);

    dPsev += protocol;
    dPsev = (dPsev & 0xFFFF) + (dPsev >> 16);

    dPsev += htons(lenCode);
    dPsev = (dPsev & 0xFFFF) + (dPsev >> 16);

    dPsev = htons(dPsev);
*/

    return dPsev;
}
