#include <stdio.h>                                                                                                                                                
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main (int argc, char* argv[]) {
    if(argc != 3) {
        printf("\nUsage:\n\t%s ip port\n",argv[0]); 
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM , IPPROTO_UDP );
    if(sock < 0) {
        perror("socket");
        return 2;
    }

    printf("sock:%d\n",sock);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons (atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr (argv[1]);

    char buf [1024];

    while(1) {
        printf("Please Enter# ");
        fflush(stdout);
        ssize_t s = read (0, buf, sizeof(buf));
        if( s > 0 ) {
            buf[s-1] = 0;
            sendto (sock, buf, strlen(buf) , 0 , (struct sockaddr*)&server, sizeof(server));
        }
    }

    close (sock);
    return 0;
}
