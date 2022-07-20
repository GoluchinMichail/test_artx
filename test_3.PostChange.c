#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pcap.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include "my_3.h"

#define MAC_DEST    "\x8c\xae\x4c\xfd\xdf\x56"
#define MAC_SRC     "\x60\xa4\x4c\xae\x90\x70"
#define DEST_IP     "10.1.2.3"
#define SRC_IP      "10.1.2.4"

static int _MyCrc (void *mem, int length) {
    int Crc = 0;
    uint8_t *bytes = (uint8_t *) mem;
    for (int i=0;i<length;i++) {
        if (i & 1) {
            Crc += ((uint16_t)bytes[i]) << 8; // как старший байт слова
        } else {
            Crc += bytes[i];
        }
    }
    Crc += Crc >> 16; // учесть накопившиеся переполнения (то что левее 16-ти бит)
    return Crc;
}

static uint16_t MyCrc (void *mem, int length) {
    int i = _MyCrc(mem,length);
    return ~ i;
}

void PostChange (void *buffer) {
    struct ethhdr *eth = (struct ethhdr *) buffer;
    struct iphdr  *ip  = (struct iphdr *)  &((char*)eth)[sizeof(*eth)];
    struct udphdr *udp = (struct udphdr *) &((char*)ip) [sizeof(*ip)];

    memcpy (eth->h_dest, MAC_DEST, 6);
    memcpy (eth->h_source,  MAC_SRC, 6);

    ip->daddr = inet_addr (DEST_IP);
    ip->saddr = inet_addr (SRC_IP);
    ip->id = htons(ip->id);
    ip->check = 0;
    ip->check = MyCrc (ip, sizeof(*ip));
    
    udp->check = 0; // не удалось правильно составить
}
