#include <stdio.h>                                                                                                                                                
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if.h>
#include <ev.h>
#include <pthread.h>
#include <ctype.h>

#include "my.h"

static void input_cb (struct ev_loop *evLoop, ev_io *w, int ) {
    _Prm_Client *prm = ev_userdata (evLoop);
    int sock_in = w->fd;
    static int iCountPacket = 0;

    int s = recv (sock_in, prm->Buffer_In, sizeof(prm->Buffer_In), 0);
    if ( s > 0 ) {
        struct ethhdr *ether = (struct ethhdr *) prm->Buffer_In;
        if (htons(ether->h_proto)==ETH_P_IP) {
            struct iphdr *ip = (struct iphdr *) ( &((char*)(ether)) [sizeof(*ether)]);
            if (ip->protocol == IPPROTO_UDP) {
                struct udphdr *udp = (struct udphdr *) ( &((char*)(ip)) [ip->ihl<<2]);
                if (
                ip->version==4 &&
                (ip->ihl<<2) == sizeof(*ip) &&
                ip->tot_len &&
                udp->len &&
                htons(ip->tot_len)== sizeof(*ip) + htons(udp->len)
                ) {

if (!prm->boBusy) {
    prm->boBusy = 1;
} else
    prm->iCountBusy++;

                    printf ("\n%u packet  BUSY= %u\n", ++iCountPacket, prm->iCountBusy);
                    printf ("   IP.saddr=    %s\n", inet_ntoa(* ((struct in_addr *) &(ip->saddr)) ));
                    printf ("   IP.tos=      %u\n", ip->tos);
                    printf ("   IP.ttl=      %u\n", ip->ttl);
                    printf ("   IP.saddr=    %s\n", inet_ntoa(* ((struct in_addr *) &(ip->saddr)) ));
                    printf ("   UDP.src=     %u\n", htons(udp->source));
                    printf ("   IP.daddr=    %s\n", inet_ntoa(* ((struct in_addr *) &(ip->daddr)) ));
                    printf ("   UDP.dest=    %u\n", htons(udp->dest));
                    int len_msg = htons(udp->len) - sizeof(*udp);
                    printf ("   UDP.src len=%u  >>", len_msg);

                    // послать ип-пакет первому потоку
                    sendto (prm->sock_client_ex, ip, htons(ip->tot_len), 0,
                        (struct sockaddr*)&prm->Share.ServerEx,sizeof(prm->Share.ServerEx)
                    );
                } else
                    ;// puts ("-ERROR format");
            } // no UDP
        } // no IP frame
    } // recv ERROR
}

static void exchange_cb (struct ev_loop *evLoop, ev_io *w, int ) {
    _Prm_Client *prm = ev_userdata (evLoop);
    struct sockaddr_un  clnt_addr;
    int caddrlen = sizeof(clnt_addr);
//    static char Buffer [MAX_SIZE_PACKET];

    int n;
    if (
        (   n = recvfrom (w->fd, prm->Buffer_Ex, sizeof(prm->Buffer_Ex), 0,
            (struct sockaddr*)&clnt_addr, &caddrlen)
        ) > 0
    ) {
        struct iphdr *ip = (struct iphdr *) prm->Buffer_Ex;
        struct udphdr *udp = (struct udphdr *) ( &((char*)(ip)) [ip->ihl<<2]);
        char *msg = & ((char*)udp) [sizeof(*udp)];

        // допечатать пакет
        int size_print = htons(udp->len) - sizeof(*udp);
        for (int i=0;i<size_print;i++) {
            if (isprint(msg[i])) putchar(msg[i]); else putchar('.');
            if (i > 10) break;
        }
        puts ("<<");

//////////////////////////////        //  ПЕРЕСЫЛКА ...
        /////////////////////   обновление CRC

        // подменить адреса дла доставки
        ip->saddr = prm->Share.resend_Src_ip;
        ip->daddr = prm->Share.resend_Dest_ip;

        // обновить все CRC
        ip->check = 0;
        ip->check = ~ _MyCrc (ip,sizeof(ip->ihl<<2));

        udp->check = 0;
        // это не просто :)
            int iUdp = _MyCrc (udp, htons(udp->len));
            int iPsev = MyPsevdoCrc (ip->saddr, ip->daddr, ip->protocol, udp->len);
            // а ип-стэк ограничивается псевдо-заголовком  - Wireshark жалуется на сумму УДП :)
            iUdp = iPsev + iUdp;
            int iMyUdp = (iUdp & 0xFFFF) + (iUdp >> 16); // учесть переполнения
        udp->check = ~ iMyUdp;

        printf ("       IP.saddr=    %s\n", inet_ntoa(* ((struct in_addr *) &(ip->saddr)) ));
        printf ("       IP.tos=      %u\n", ip->tos);
        printf ("       IP.ttl=      %u\n", ip->ttl);
        printf ("       IP.saddr=    %s\n", inet_ntoa(* ((struct in_addr *) &(ip->saddr)) ));
        printf ("       UDP.src=     %u\n", htons(udp->source));
        printf ("       IP.daddr=    %s\n", inet_ntoa(* ((struct in_addr *) &(ip->daddr)) ));
        printf ("       UDP.dest=    %u\n", htons(udp->dest));
        int len_msg = htons(udp->len) - sizeof(*udp);
        printf ("   UDP.src len=%u  >>", len_msg);
        size_print = htons(udp->len) - sizeof(*udp);
        for (int i=0;i<size_print;i++) {
            if (isprint(msg[i])) putchar(msg[i]); else putchar('.');
            if (i > 10) break;
        }
        puts ("<<");

        /////////////////////   ПОСЫЛКА
        // ну и зачем "sockaddr_in", в буфере "ip" всё это уже есть %/
        struct sockaddr_in Sin;
        memset (&Sin, -1, sizeof(Sin));
        Sin.sin_family = AF_INET;
        //  Sin.sin_port = htons (udp_dest);     NOT USE %/
        Sin.sin_addr.s_addr = ip->daddr;
        if (
            sendto(prm->Share.sock_out, ip, htons(ip->tot_len), 0, (struct sockaddr *)&Sin, sizeof(Sin))
            < 0
        )
            perror("send");
        else
            puts ("   -resend OK");
//////////////////////////////        //  ПЕРЕСЫЛКА ...

    }
if (prm->boBusy)
    prm->boBusy = 0;
}

void* thread_2 (void* args) {
    _Prm_Client *prm = malloc (sizeof(_Prm_Client));
    if (!prm) {
        perror ("-ERROR  malloc");
        return NULL;
    }
    memcpy (&prm->Share, args, sizeof(prm->Share));

    prm->boBusy = 0;
    prm->iCountBusy = 0;

    // свой сокет для обмена с первым потоком
    prm->sock_client_ex = GetUnixSock (prm->Share.ServerEx.sun_path, NULL);
    if (prm->sock_client_ex >= 0) {
        // добавить обработчик
        struct ev_loop *evloop_2 = ev_loop_new (EVFLAG_AUTO);
        if (evloop_2) {
            ev_io w_Input;
            ev_io_init (&w_Input, input_cb, prm->Share.sock_in, EV_READ);
            ev_io_start (evloop_2, &w_Input);
            ev_io w_Exchange;
            ev_io_init (&w_Exchange, exchange_cb, prm->sock_client_ex, EV_READ);
            ev_io_start (evloop_2, &w_Exchange);
  
            ev_set_userdata (evloop_2, prm);
            ev_loop (evloop_2,0);

            ev_loop_destroy (evloop_2);
        } else
            perror ("-ERROR  ev_loop_new()");
        close (prm->sock_client_ex);
    } else
        perror ("-ERROR GetUnixSock()");

    free (prm);
}
