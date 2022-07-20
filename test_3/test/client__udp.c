#include <stdio.h>                                                                                                                                                
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main (int argc, char* argv[]) {
    if(argc != 3) {
        printf("Usage:%s ip port\n",argv[0]); 
        return 1;
    }

    // 1. Создать сокет
    int sock = socket(AF_INET, SOCK_DGRAM , 0 );// SOCK_DGRAM означает UDP
    if(sock < 0) {
        perror("socket");
        return 2;
    }

    printf("sock:%d\n",sock);

  // 2. Именованный сокет

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons (atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr (argv[1]);

    // 3. Клиент должен стараться не привязывать номер порта
    char buf [1024];// Буфер

    // 4. Клиент отправляет информацию

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
