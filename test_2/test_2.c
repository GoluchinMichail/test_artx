
//  gcc  -o test_2  -lev    test_2.c   test_2.Thread.c

#include "my_2.h"

static _ParamThread Prm;

static int socket_init (int iPort) {
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

static void CallBack_1 (EV_P_ ev_async *w, int revents) {
    //printf ("CallBack_1  revents= %Xh\n", revents);

    if (!memcmp(Prm.acBuffer,"+++",3)) {
        // puts ("STOPPED_1");
        ev_async_stop (Prm.loop_1, &Prm.watcher_1);
    } else {
        strrev (Prm.acBuffer);
        pthread_mutex_unlock (&Prm.LockBuffer);
    }
}

int main (int argc, char** argv) {
    if (argc < 2) {
        printf ("\n"
            "Usage:\n"
            "   %s num_port\n"
            "\n"
            , argv[0]
        );
        return -1;
    }

    // Initialize pthread
    Prm.iSocket = socket_init (atoi(argv[1]));

    puts ("press Ctrl-C to exit ...\n");

    pthread_mutex_init (&Prm.LockBuffer, NULL);
    Prm.loop_1 = ev_loop_new (0);
    ev_async_init (&Prm.watcher_1, CallBack_1);
    ev_async_start (Prm.loop_1, &Prm.watcher_1);
    pthread_t thread;
    pthread_create (&thread, NULL, thread_2, &Prm);

    puts ("LOOP_1 >>>>>>>>>>>>>>>>>>>>>>");
    // now wait for events to arrive
    ev_loop (Prm.loop_1, 0);
    puts ("LOOP_1 <<<<<<<<<<<<<<<<<<<<<<");

    puts ("Wait on threads for execution ...");
    pthread_join (thread, NULL);

    close (Prm.iSocket);
    pthread_mutex_destroy (&Prm.LockBuffer);

    puts ("\n... BY");
    return 0;
}
