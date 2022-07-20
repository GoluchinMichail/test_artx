//
//      gcc  -o test_1 -lev  test_1.c
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <errno.h>
#include <arpa/inet.h>
#include <ev.h>

char acBuffer [1024];

int socket_init (int iPort) {
    struct sockaddr_in my_addr;
    int listener;
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("socket");
        exit (1);
    } else
        printf ("SOCKET CREATE SUCCESS!\n");

    int so_reuseaddr=1;
    setsockopt (listener,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
    memset (&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(iPort);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
    if (bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
        perror ("bind error!\n");
        exit (1);
    } else
        puts ("IP BIND SUCCESS,IP: 127.0.0.1");
 
    if (listen(listener, 1024) == -1) {
        perror ("listen error!\n");
        exit (1);
    } else
        printf ("LISTEN SUCCESS, PORT: %d\n\n", iPort);

    return listener;
}

void recv_callback (struct ev_loop *evLoop, ev_io *w, int );

void write_callback (struct ev_loop *evLoop, ev_io *w, int ) {
    write (w->fd,acBuffer,strlen(acBuffer));

    ev_io_stop (evLoop,  w);
    ev_io_init (w,recv_callback,w->fd,EV_READ);
    ev_io_start (evLoop,w);
}

void recv_callback (struct ev_loop *evLoop, ev_io *w, int ) {

    int ret = recv (w->fd,acBuffer,sizeof(acBuffer),0);
    if (ret > 0) {
        acBuffer[ret] = 0;
        printf (acBuffer);

        // послать ответ ... тот что в  prm->acBuffer
        ev_io_stop (evLoop,  w);
        ev_io_init (w, write_callback, w->fd, EV_WRITE);
        ev_io_start (evLoop, w);

    } else {
        puts ("remote socket closed!");
        ev_io_stop (evLoop, w);

        // как ещё остановить это - итио ^_^     извините :)
        ev_io_init (w,NULL,0,0);
        ev_io_stop (evLoop, w);

    }

}

void accept_callback (struct ev_loop *evLoop, ev_io *w, int ) {
    int newfd;
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(struct sockaddr);

    while ((newfd = accept(w->fd, (struct sockaddr *)&sin, &addrlen)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue; 
        } else {
            printf ("accept error.[%s]\n", strerror(errno));
            break;
        }
    }

    ev_io_stop (evLoop,w);
    ev_io_init (w,recv_callback,newfd,EV_READ);
    ev_io_start (evLoop,w);

    printf("accept callback : fd :%d\n",w->fd);
}

int main (int argc ,char** argv) {
    if (argc < 2) {
        printf ("\n"
"Usage:\n"
"   %s num_port\n"
        "\n", argv[0]);
        return 1;
    }

    int iSockListen = socket_init (atoi(argv[1]));

    puts ("press Ctrl-C to exit ...\n");

    struct ev_loop *evloop = ev_loop_new (EVBACKEND_EPOLL);

    ev_io ev_io_watcher;

    ev_io_init (&ev_io_watcher, accept_callback,iSockListen, EV_READ);

    ev_io_start (evloop,&ev_io_watcher); 
    ev_loop (evloop,0);

    ev_loop_destroy (evloop);
    close (iSockListen);

    puts ("... BY");
    return 0;
}
