#include <stdio.h>                                                                                                                                                
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main (int argc, char* argv[]) {
    if(argc < 2) {
        printf("Usage:\n\t%s  port\n",argv[0]);
        return 1;
    }

    // 1. Создать сокет

    int sock = socket(AF_INET, SOCK_DGRAM , 0 );// SOCK_DGRAM означает UDP
//    int sock = socket(AF_INET, SOCK_RAW , IPPROTO_UDP);
    if(sock < 0) {
        perror ("socket");
        return 2;
    }
    printf("Socker: %d\n",sock);


    // 2. Именованный сокет

    struct sockaddr_in local;
    memset (&local,0,sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(atoi(argv[1]));

    // 3. Привязать номер порта
    if( bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0  ) {
        perror ("bind");
        return 3;
    }

    u_char buf [1024];// Буфер

    // 4. Сервер получает информацию о клиенте

    while(1) {
        struct sockaddr_in client;

        socklen_t len = sizeof(client);
        memset (buf,0,sizeof(buf));
        ssize_t s = recvfrom (sock, buf, sizeof(buf)-1, 0,(struct sockaddr*)&client,&len);
        if ( s > 0 ) {
            buf[s] = 0;
            printf ("%u bytes  %s port %d ==> %s\n", s, inet_ntoa(client.sin_addr), ntohs(client.sin_port), buf);
            puts (&buf[20+8]);
        }
    }

    close (sock);
    return 0;
}
