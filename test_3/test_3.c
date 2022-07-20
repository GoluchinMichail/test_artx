
//
//  gcc  -o test_3  -lev -lpcap    test_3.c   test_3.Thread.c   test_3.Processing.c

#include "my_3.h"

static _ParamThread Prm;

static char *memrev (char *p, int length) {
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
    
    struct ethhdr *eth = (struct ethhdr *) Prm.buffer;
    struct iphdr  *ip  = (struct iphdr *)  &((char*)eth)[sizeof(*eth)];
    struct udphdr *udp = (struct udphdr *) & ((char*)ip) [sizeof(*ip)];

    char *src = & ((char *)udp) [sizeof(*udp)];

    memrev (src, htons(udp->len) - sizeof(*udp));

    void PostChange (void *);
    PostChange (Prm.buffer);

    // пометить блок "Prm.buffer" доступным
    pthread_mutex_unlock (&Prm.LockBuffer);
}

int main (int argc, char** argv) {
    if (argc < 3) {
Help:
        printf (
"\n"
"Usage:\n"
"   %s  inFace  outFace\n"
"\n"
"Sample:\n"
"   %s  lo  eth0"
"\n"
        , argv[0], argv[0]);
        return -1;
    }

    if (!memcmp(argv[1],argv[2],strlen(argv[2])+1)) {
        printf ("\n-одинаковые IN/OUT - будет гонка !\n");
        goto Help;
    }


    // Initialize pthread
    Prm.argv_1 = argv[1];
    Prm.argv_2 = argv[2];

    puts ("\npress Ctrl-C to exit ...\n");

    pthread_mutex_init (&Prm.LockBuffer, NULL);
    Prm.loop_1 = ev_loop_new (0);
    ev_async_init (&Prm.watcher_1, CallBack_1);
    ev_async_start (Prm.loop_1, &Prm.watcher_1);
    pthread_t thread;
    pthread_create (&thread, NULL, thread_3, &Prm);

    puts ("LOOP_1 >>>>>>>>>>>>>>>>>>>>>>");
    // now wait for events to arrive
    ev_loop (Prm.loop_1, 0);
    puts ("LOOP_1 <<<<<<<<<<<<<<<<<<<<<<");

    puts ("Wait on threads for execution ...");
    pthread_join (thread, NULL);

    pthread_mutex_destroy (&Prm.LockBuffer);

    puts ("\n... BY");
    return 0;
}
