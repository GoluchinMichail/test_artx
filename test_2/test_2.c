//
//      gcc  -o test_2 -lev  test_2.c
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <errno.h>
#include <arpa/inet.h>
#include <ev.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stddef.h>

#include "my_2.h"
#define DEFAUL_UNIX_SOCKETS "/tmp/test2_"

static char *strrev (char *p) {
    // исключить EOL-символы
    int length = strcspn (p, "\r\n");
    if (length) {
        int c, i, j;
        for (i=0, j=length - 1; i < j; i++, j--) {
            c = p[i];
            p[i] = p[j];
            p[j] = c;
        }
    }

    return p;
}

static int sock_bind_listen (int iPort) {
    struct sockaddr_in my_addr;
    int listener;

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("socket");
        return -1;
    }

    int so_reuseaddr = 1;
    setsockopt (listener,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
    memset (&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(iPort);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
    if (bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
        perror ("bind error!\n");
        return -2;
    }
 
    if (listen(listener, 1024) == -1) {
        perror ("listen error!\n");
        return -3;
    } else
        printf ("\nLISTEN SUCCESS, PORT: %d\n\n", iPort);

    return listener;
}

static void exchange_cb (struct ev_loop *evLoop, ev_io *w, int ) {
    char Buffer [1024];
    struct sockaddr_un  clnt_addr;
    int caddrlen;

    caddrlen = sizeof(clnt_addr);
    int n = recvfrom (w->fd, Buffer, sizeof(Buffer), 0,
        (struct sockaddr*)&clnt_addr, &caddrlen);
    if (n > 0) {
        Buffer[n] = 0;
printf ("1< %s", Buffer);
        strrev (Buffer);
        int i = sendto (w->fd, Buffer, n, 0,
            (struct sockaddr*)&clnt_addr, caddrlen);
printf ("   1> %s", Buffer);
    }
}

static void accept_cb (struct ev_loop *evLoop, ev_io *w, int ) {
    int iNewSock;

    if ((iNewSock = accept(w->fd, NULL, 0)) > 0) {
        _Prm * prm = ev_userdata (evLoop);
        prm->iSock = iNewSock;
        // запуск потока для клиента
        pthread_t thread;
        pthread_create (&thread, NULL, thread_2, prm);
    }
}

int main (int argc ,char** argv) {
    if (argc < 2) {
Help:
        printf ("\n"
"Usage:\n"
"   %s num_port [FileUnixSockets]\n"
        "\n", argv[0]);
        return 1;
    }

    int port = atoi (argv[1]);
    if (! (port && port<=65535) ) {
        puts ("\n-ERROR port\n");
        goto Help;
    }
    char FileUnixSockets[] = DEFAUL_UNIX_SOCKETS;
    if (argc > 2)
        strncpy (FileUnixSockets, argv[2], sizeof(FileUnixSockets)-1);
    
    int iSockListen = sock_bind_listen (port);
    if (iSockListen > 0) {

        // printf ("iSockListen= %i\n", iSockListen);

        _Prm Prm;
        int iSockExchange =  GetUnixSock (FileUnixSockets, &Prm.unServerAddress);
        if (iSockExchange < 0) {
            printf("Невозможно создать сокет\n");
            return (1);
        }

        struct ev_loop *evloop_1 = ev_loop_new (EVFLAG_AUTO);
        ev_io w_Exchange;
        ev_io_init (&w_Exchange, exchange_cb, iSockExchange, EV_READ);
        ev_io_start (evloop_1, &w_Exchange); 
        // добавить обработчик
        ev_io *new_client = calloc (1, sizeof(*new_client));
        ev_io_init (new_client, accept_cb, iSockListen, EV_READ);
        ev_io_start (evloop_1, new_client);

        puts ("press Ctrl-C to exit ...\n");

        ev_set_userdata (evloop_1, &Prm);
        ev_loop (evloop_1,0);

        ev_loop_destroy (evloop_1);
        close (iSockListen);
        unlink (Prm.unServerAddress.sun_path);

        puts ("... BY");
    } else
        puts ("-ERROR bind");

    return 0;
}
