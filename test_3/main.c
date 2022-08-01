#include <stdio.h>                                                                                                                                                
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if.h>
#include <pthread.h>
#include <time.h>       //  clock() псевдослучайное число :)
#include <ev.h>
#include "my.h"


#define START_US_NAME   "/tmp/test_3__"

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

static void exchange_cb (struct ev_loop *evLoop, ev_io *w, int ) {
    struct sockaddr_un  clnt_addr;
    int addrlen = sizeof(clnt_addr);
    void *buf = ev_userdata (evLoop); //    MAX_SIZE_PACKET

    int n = recvfrom (w->fd, buf, MAX_SIZE_PACKET, 0,
        (struct sockaddr*)&clnt_addr, &addrlen);
    if (n > 0) {
        struct iphdr *ip = (struct iphdr *) buf;
        struct udphdr *udp = (struct udphdr *) ( &((char*)(ip)) [ip->ihl<<2]);
        char *msg = & ((char*)udp) [sizeof(*udp)];
        int len_msg = htons(udp->len) - sizeof(*udp);
        memrev (msg, len_msg);
        // и послать обратно ...
        sendto (w->fd, ip, n, 0,
            (struct sockaddr*)&clnt_addr, addrlen);
    }
}
   
int main (int argc, char* argv[]) {
    if (argc < 1+  4) {
Help:
        printf (
"\nUsage :\n"
"\tsudo  %s  control_IF   resend_IF    resend_IP_Src    resend_IP_Dest\n"
"\n"
"Sample:\n"
"\tsudo  %s     lo          eth0       10.1.2.3         10.1.2.4\n"
            , argv[0], argv[0]
        );
        return 1;
    }
    char *control_if = argv[1];
    char *resend_if = argv[2];
    char *resend_MY_ip = argv[3];
    char *resend_ip = argv[4];

////////////////////////////////////////////////////////////////////////////////////////////
    // ВХОДЯЩИЙ ИНТЕРФЕЙС
    int sock_in = socket (PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if(sock_in < 0) {
        perror ("-ERROR  socket_in");
        return 1;
    }
    // нацеливание сокета на заданный интерфейс ...
    struct ifreq ifr;
    strncpy (ifr.ifr_name, control_if, sizeof(ifr.ifr_name));
    if (setsockopt(sock_in, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror ("-ERROR  setsockopt() for SO_BINDTODEVICE");
        return 1;
    }

////////////////////////////////////////////////////////////////////////////////////////////
    // ВЫХОДНОЙ ИНТЕРФЕЙС
    int sock_out = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock_out < 0 ) {
        perror("socket_out");
        return 1;
    }
    int one = 1;
    const int *val = &one;
    //  наполнять ип-пакет буду самостоятельно
    if (setsockopt(sock_out, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror ("-ERROR setsockopt ( IP_HDRINCL )");
        return 1;
    }
    // нацеливание сокета на заданный интерфейс
    strncpy (ifr.ifr_name, resend_if, sizeof(ifr.ifr_name));
    if (setsockopt(sock_out, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror ("-ERROR  setsockopt() for SO_BINDTODEVICE");
        return 1;
    }
////////////////////////////////////////////////////////////////////////////////////////////

    _Prm_Server Prm_Server;
    Prm_Server.sock_in = sock_in;
    Prm_Server.sock_out = sock_out;
    Prm_Server.resend_Src_ip = inet_addr (resend_MY_ip);
    Prm_Server.resend_Dest_ip = inet_addr (resend_ip);
    
    int sock_server_ex = GetUnixSock (START_US_NAME, &Prm_Server.ServerEx);
    if (sock_server_ex < 0) {
        perror ("-ERROR  GetUnixSock()");
        return 1;
    }
    
    struct ev_loop *evloop_1 = ev_loop_new (EVFLAG_AUTO);
    if (evloop_1) {
        ev_io w_Input;
        ev_io_init (&w_Input, exchange_cb, sock_server_ex, EV_READ);
        ev_io_start (evloop_1, &w_Input);

        pthread_t thread;
        pthread_create (&thread, NULL, thread_2, &Prm_Server);
  
        char Buffer [MAX_SIZE_PACKET];
        ev_set_userdata (evloop_1, Buffer);

        puts ("wait ...  Press Ctrl-C to exit ...");
        ev_loop (evloop_1,0);

        // формальность ...
        ev_loop_destroy (evloop_1);
        close (Prm_Server.sock_in);
        close (Prm_Server.sock_out);
        close (sock_server_ex);
    } else
        perror ("-ERROR  ev_loop_new()");

    return 0;
}
