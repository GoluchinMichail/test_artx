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

    int sock = socket(AF_INET, SOCK_DGRAM , IPPROTO_UDP );
    if(sock < 0) {
        perror ("socket");
        return 2;
    }
    printf("Socker: %d\n",sock);


    struct sockaddr_in local;
    memset (&local,0,sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(atoi(argv[1]));

    if( bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0  ) {
        perror ("bind");
        return 3;
    }

    u_char buf [1024];// Буфер

    while(1) {
        struct sockaddr_in client;

        socklen_t len = sizeof(client);
        memset (buf,0,sizeof(buf));
        ssize_t s = recvfrom (sock, buf, sizeof(buf)-1, 0,(struct sockaddr*)&client,&len);
        if ( s > 0 ) {
            buf[s] = 0;
            printf ("%u bytes  %s port %d ==> %s\n", s, inet_ntoa(client.sin_addr), ntohs(client.sin_port), buf);
        }
    }

    close (sock);
    return 0;
}
