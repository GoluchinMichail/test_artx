#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pcap.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include "my_3.h"

static pcap_t *indev = NULL, *outdev = NULL;
static _ParamThread *prm;

static void my_callback (u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet) { 

    struct ethhdr *eth = (struct ethhdr *) packet;

    if (htons(eth->h_proto) == ETH_P_IP) {
        struct iphdr *ip = (struct iphdr *) &packet[sizeof(*eth)];
        if (ip->protocol==IPPROTO_UDP) {
            struct udphdr *udp = (struct udphdr *) & ((char*)ip) [sizeof(*ip)];
  
            char *src = (char *)udp + sizeof(*udp);
            printf ("         Src: %s\n", src);

            // сообщить 1-му потоку о доступности буфера
            prm->buffer = (char *) eth;
            // "первый" свободен ?
            if (ev_async_pending(&prm->watcher_1)==0) {
                pthread_mutex_lock (&prm->LockBuffer);  // пусть 1-й поток и разблокирует это
                ev_async_send (prm->loop_1, &prm->watcher_1);
            }

            // дождаться обработки строки в 1-м потоке
            pthread_mutex_lock (&prm->LockBuffer); // пусть 1-й поток разблокирует это
            pthread_mutex_unlock (&prm->LockBuffer);

            printf ("      newSrc: %s\n", src);

            int res = pcap_inject (outdev, prm->buffer, pkthdr->len);
            printf ("   pcap_inject= %i  %s\n", res, (res>0?"-OK":"-ERROR"));

        }
    }
}

static pcap_t *GetDevice (char *name, void *buffer) {
    pcap_t *devRes = NULL;

    pcap_if_t *listDevs;

    if (pcap_findalldevs(&listDevs, buffer) == -1 || !listDevs) {
        fprintf(stderr, "No network devices are currently connected\n");
        return NULL;
    }

    pcap_if_t *iface = listDevs;
    for (int i = 1; iface; iface = iface->next) {
        // printf ("%d - %s - %s\n", i++, iface->name, iface->description);
        if (!memcmp(iface->name,name,strlen(name)+1))
            break;
    }
    if (iface) {
        printf ("device \"%s\" -OK\n", name);
        if ((devRes = pcap_open_live(iface->name,    // name of the device
                             65536,             // portion of the packet to capture. It doesn't matter in this case
                             0,                 // promiscuous mode (nonzero means promiscuous)
                             1,                 // read timeout
                             buffer             // error buffer
        )) == NULL) {
            fprintf (stderr, "\nUnable to open the adapter \"%s\", is not supported by Pcap  (not root ?)\n", name);
        }
    } else
        printf ("device \"%s\" not presence\n", name);

    // теперь listDevs можно освободить
    pcap_freealldevs (listDevs);
    return devRes;
}

void * thread_3 (void* args) {
    puts ("THREAD_2 >>>>>>>>>>>>>>>>>>>>>>>");
    prm = args;

    int iResult = 0;
    do {

    char InBuff [PCAP_ERRBUF_SIZE];
    indev = GetDevice (prm->argv_1, InBuff);
    if (!indev)
        break;
    char OutBuff [PCAP_ERRBUF_SIZE];
    outdev = GetDevice (prm->argv_2, OutBuff);
    if (!outdev) {
        pcap_close (indev);
        break;
    }

    //   выражение фильтрации
    struct bpf_program Filter;
    #define NO_OPTIMIZE    0
    #define STR_FILTER  ""  //   "port 8003"
    if (pcap_compile(indev, &Filter, STR_FILTER, NO_OPTIMIZE, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf (stderr, "Error calling pcap_compile \"%s\"\n", STR_FILTER);
        break;
    }

    //  применение фильтра
    if (pcap_setfilter(indev, &Filter) == -1) {
        fprintf (stderr, "Error setting filter\n");
        break;
    } 

    puts ("loop ...");
    pcap_loop (indev, -1, my_callback, NULL);

    } while (0);

    if (indev)
        pcap_close (indev);
    if (outdev)
        pcap_close (outdev);

    puts ("THREAD_2 <<<<<<<<<<<<<<<<<<<<<<<");
    return NULL;
}
