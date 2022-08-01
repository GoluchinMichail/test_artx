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

int sock_bind_listen (int iPort) {
    struct sockaddr_in my_addr;
    int listener;

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("socket");
        return -1;
    } else
        printf ("SOCKET CREATE SUCCESS!\n");

    int so_reuseaddr = 1;
    setsockopt (listener,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
    memset (&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(iPort);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
    if (bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
        perror ("bind error!\n");
        return -2;
    } else
        puts ("IP BIND SUCCESS,IP: 127.0.0.1");
 
    if (listen(listener, 1024) == -1) {
        perror ("listen error!\n");
        return -3;
    } else
        printf ("LISTEN SUCCESS, PORT: %d\n\n", iPort);

    return listener;
}

void recv_cb (struct ev_loop *evLoop, ev_io *w, int ) {

    char acBuffer [1024];
    int ret = recv (w->fd,acBuffer,sizeof(acBuffer)-1,0);
    if (ret > 0) {
        acBuffer [ret] = 0;
        printf (acBuffer);
        write (w->fd, strrev(acBuffer),strlen(acBuffer));
    } else if ((ret < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        ;
    } else {
        puts ("remote socket closed!");
        close (w->fd);
        // отменить текущий обработчик
        ev_break (evLoop, EVBREAK_CANCEL);
        // и освободить ресурс
        ev_io_stop (evLoop, w);
        free (w);
    }

}

void accept_cb (struct ev_loop *evLoop, ev_io *w, int ) {
    int newSock;

    if ((newSock = accept(w->fd, NULL, 0)) > 0) {
        // добавить новый обработчик
        ev_io *new_client = calloc (1, sizeof(*new_client));
        ev_io_init (new_client, recv_cb, newSock, EV_READ);
        ev_io_start (evLoop, new_client);
        puts ("new client ...");
    }
}

int main (int argc ,char** argv) {
    if (argc < 2) {
Help:
        printf ("\n"
"Usage:\n"
"   %s num_port\n"
        "\n", argv[0]);
        return 1;
    }

    int port = atoi (argv[1]);
    if (! (port && port<=65535) ) {
        puts ("\n-ERROR port\n");
        goto Help;
    }
    
    int iSockListen = sock_bind_listen (port);
    if (iSockListen > 0) {
        struct ev_loop *evloop = ev_default_loop (EVFLAG_NOENV);
        ev_io *watcher = calloc (1, sizeof(*watcher));
        if (evloop && watcher) {
            ev_io_init (watcher, accept_cb, iSockListen, EV_READ);
            ev_io_start (evloop, watcher);
            puts ("press Ctrl-C to exit ...\n");
            ev_run (evloop, 0);

            ev_loop_destroy (evloop);
            free (watcher);
        } else 
            puts ("-ERROR  small memory\n");
    }

    puts ("... BY");
    return 0;
}
