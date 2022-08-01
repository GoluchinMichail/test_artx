#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/un.h>
#include <time.h>

int GetUnixSock (char *pre_name, struct sockaddr_un *unServerAddress) {
    int sockfd;

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) >= 0) {
        struct sockaddr_un un_addr;
        memset (&un_addr, 0, sizeof(un_addr));
        un_addr.sun_family = AF_UNIX;

        snprintf (un_addr.sun_path, sizeof(un_addr.sun_path)-1, "%s__%u", pre_name, clock());
        unlink (un_addr.sun_path);

        int caddrlen =
            sizeof(un_addr.sun_family) + strlen(un_addr.sun_path);
        if (bind(sockfd, (struct sockaddr*)&un_addr, sizeof(un_addr)/*caddrlen*/) < 0) {
            printf ("Ошибка связывания сокета\n");
            close (sockfd);
            sockfd = -1;
        } else if (unServerAddress)
            memcpy (unServerAddress,&un_addr,sizeof(*unServerAddress));
        
    } else
        printf ("Невозможно создать сокет\n");

    return sockfd;
}
