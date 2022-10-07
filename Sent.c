#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>  // FreeBSD
#include <arpa/inet.h>
#include <ev.h>
#include <time.h>

// #include "kbhit.h"
#include "packet.h"

typedef struct {
    int sock;
    struct sockaddr_in Sin;

    int iCounter_Tick_10;
    int iCounter_Sleep;
    _Packet Packet;
} _Param;

/*
static void console_cb (struct ev_loop *evLoop, ev_io *w, int revents) {
    if (console_getkey()==0x1B) {
        ev_break (evLoop, EVBREAK_ALL);
        puts ("  -BREAK CONSOLE");
    }
}
*/

static void timer_10_cb (struct ev_loop *loop, ev_timer *w, int revents);

static void timer_Sleep_cb (struct ev_loop *loop, ev_timer *w, int revents) {
    // запустить посылку
    puts ("<<<<<<<< SLEEP");
    ev_timer_init (w, timer_10_cb, PERIOD_SECONDS, PERIOD_SECONDS);
    ev_timer_start (loop, w);
}

static void FillBuffer (uint16_t *buffer, int Words) {
    //  предполагая что используется кодировка UTF-8  !!
    static char Str[] =
        "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"
        "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
    uint16_t *s16 = (uint16_t *) Str;
    for (int i=0,j=0; i<Words; i++) {
        buffer[i] = s16[j];
        j++;
        if (j>=sizeof(Str)/sizeof(uint16_t))
            j = 0;
//buffer[i+1] = 0;
//printf ("%1s", &buffer[i]);
    }
//puts ("");
}

static void timer_10_cb (struct ev_loop *loop, ev_timer *w, int revents) {
    _Param *param = ev_userdata (loop);

    if (param->iCounter_Tick_10 < 1000) {
        // посылка

        _Packet *packet = &param->Packet;
        packet->Head.iNumber++;
        packet->Head.iWords = 600;
        // наращивание размера пакета
        packet->Head.iWords += param->iCounter_Tick_10;
        // паранойя :) вдруг обсчитался :)
        int Bytes = packet->Head.iWords * sizeof(packet->aBuffer[0]);
        if (Bytes > sizeof(packet->aBuffer)) {
            printf ("-ERROR size buffer = %u\n", Bytes);
            ev_break (loop, EVBREAK_ALL);   // на выход
            return;
        }

        // наполнить пакет
        FillBuffer (packet->aBuffer, packet->Head.iWords);

        MD5_CTX     MD5;
        MD5_Init   (&MD5);
        MD5_Update (&MD5, packet->aBuffer, packet->Head.iWords * sizeof(packet->aBuffer[0]));
        MD5_Final  (packet->Head.Md5, &MD5);
        clock_t stamp = clock();
        printf ("Sent: %2u  %u   %u words\n", packet->Head.iNumber, (uint32_t)stamp, packet->Head.iWords);
// порча crc
//if (1 & param->iCounter_Sent)
//Packet.aBuffer[13] ++;
        int err = sendto (param->sock, packet, sizeof(packet->Head) + (packet->Head.iWords * sizeof(packet->aBuffer[0])), 0, (struct sockaddr*)&param->Sin, sizeof(param->Sin));
        if (err<=0)
            perror ("sendto");
        param->iCounter_Tick_10++;
    } else {
        param->iCounter_Tick_10 = 0;
        // остановить посылку
        ev_timer_stop (loop, w);

        if (!param->iCounter_Sleep) {
            param->iCounter_Sleep ++;     //  в следующий раз - на выход
            ev_set_userdata (loop,param);

            param->iCounter_Tick_10 = 0;

            // заснуть на SLEEP_SECONDS один раз
            ev_timer_init (w, timer_Sleep_cb, SLEEP_SECONDS, 0);
            ev_timer_start (loop, w);
            //printf ("SLEEP at %u seconds ...\r", SLEEP_SECONDS); fflush(stdout);
        } else {
            ev_break (loop, EVBREAK_ALL);   // всё - на выход
            //puts ("--BREAK");
        }
    } // if (param->iCounter_Tick_10 < 10) else ...
}

int main (int argc, char** argv) {

    if (argc < 1+  2) {
Help:
        printf (
"\nUsage :\n"
"\t%s   ip  port\n"
"\n"
"Sample:\n"
"\t%s   127.0.0.1   1234\n"
            , argv[0], argv[0]
        );
        return 1;
    }

    _Param Param;
    memset (&Param,0,sizeof(Param));

    Param.sock = socket(AF_INET, SOCK_DGRAM , IPPROTO_UDP );
    if(Param.sock < 0) {
        perror("socket");
        return 2;
    }
    Param.Sin.sin_family = AF_INET;
    Param.Sin.sin_port = htons (atoi(argv[2]));
    Param.Sin.sin_addr.s_addr = inet_addr (argv[1]);

    // Создание основного цикла для обработки событий
    struct ev_loop *loop = ev_default_loop (EVFLAG_AUTO);

//    // консоль Esc etc
//    ev_io w_Cancel;
//    ev_io_init (&w_Cancel, console_cb, 0, EV_READ);
//    ev_io_start (loop, &w_Cancel);

    // таймер по 10 мс
    struct ev_timer timer_ev;
    ev_timer_init (&timer_ev, timer_10_cb, PERIOD_SECONDS, PERIOD_SECONDS);
    ev_timer_start (loop, &timer_ev);

    ev_set_userdata (loop,&Param);
//    int iSave = install_kbhit ();
//puts ("Press Esc to exit ...");
    ev_run(loop, EVFLAG_AUTO);
    // закрытие ресурсов ...
    ev_loop_destroy (loop);
    close (Param.sock);
//    close_kbhit (iSave);
//puts ("--EXIT from Main()");

    return 0;
}
