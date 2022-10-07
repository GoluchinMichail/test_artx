#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>	//	FreeBSD
#include <arpa/inet.h>
#include <ev.h>

#include "ring_buffer.h"
#include "received-processed.h"

static void sigint_callback (struct ev_loop *, ev_signal *, int);

static void input_cb (struct ev_loop *evLoop, ev_io *w, int not_use) {
    char Buffer[sizeof(_Packet)];
    _Packet *packet = (_Packet *) Buffer;
    _Param_Processed *param = ev_userdata (evLoop);

    struct sockaddr_in Client;
    socklen_t iLen = sizeof(Client);
    int s = recvfrom (w->fd, Buffer, sizeof(Buffer), 0, (struct sockaddr*)&Client,&iLen);
    if ( s > 0 && s==sizeof(packet->Head) + (packet->Head.iWords * sizeof(packet->aBuffer[0]))) {

        if (s > sizeof(packet->Head)) {
            
            MD5_CTX     MD5;
            MD5_Init   (&MD5);
            MD5_Update (&MD5, packet->aBuffer, packet->Head.iWords * sizeof(packet->aBuffer[0]));
            uint8_t     new_md5 [MD5_DIGEST_LENGTH];
            MD5_Final  (new_md5, &MD5);

            int imd5 = memcmp (packet->Head.Md5,new_md5,sizeof(new_md5));

            printf ("Received: %3u   %u  %s    words %4u\n", packet->Head.iNumber, (uint32_t)clock(), (!imd5 ? "PASS" : "FAIL"), packet->Head.iWords);

            WriteToRingBuffer (&param->Ring, packet);
            //int value;
            //sem_getvalue (&param->semaRing, &value);
            //if (!value)
            sem_post (&param->semaRing);

        } else
            puts ("-ERROR size packet");
    } else
        perror ("recvfrom");
}

static void sigint_callback(struct ev_loop *loop, ev_signal *w, int revents) {
    puts ("the Ctrl-C is pressed");

	fflush (stdout);

    exit(0);
}

int main (int argc, char** argv) {

    if (argc < 1+  2) {
Help:
        printf (
"\nUsage :\n"
"\t%s   port   sleep_ms\n"
"\n"
"Sample:\n"
"\t%s   1234   15\n"
            , argv[0], argv[0]
        );
        return 1;
    }

    uint iSleep_MS = atoi(argv[2]);
    //if (!iSleep_MS)   ноль тоже интересен
    //    goto Help;

    int sock = socket(AF_INET, SOCK_DGRAM , IPPROTO_UDP);
    if(sock < 0) {
        perror("socket");
        return 2;
    }
    // привязаться к порту
    struct sockaddr_in Sin;
    memset (&Sin, 0, sizeof(Sin));
    Sin.sin_family = AF_INET;
    Sin.sin_port = htons (atoi(argv[1]));
    if (bind(sock, (struct sockaddr *) &Sin, sizeof(Sin))== -1) {
        perror ("bind error!\n");
        return -2;
    }

    // Создание основного цикла для обработки событий
    struct ev_loop *loop = ev_default_loop (EVFLAG_AUTO);

	// При нажатии Ctrl+C - сбросить буфер стандартного вывода, для целостности журнала
    struct ev_signal sigint_sig;
    ev_signal_init (&sigint_sig, sigint_callback, SIGINT);
    ev_signal_start (loop, &sigint_sig);

    //  сетевое
    ev_io w_Input;
    ev_io_init (&w_Input, input_cb, sock, EV_READ);
    ev_io_start (loop, &w_Input); 

    _Param_Processed Param = {0,iSleep_MS};
    InitRingBuffer (&Param.Ring);

    ev_set_userdata (loop,&Param);

    sem_init (&Param.semaRing, 0, 0);

    //puts ("Press Esc to exit ...");
    pthread_t thread;
    pthread_create (&thread, NULL, Processed, &Param);

    ev_run(loop, EVFLAG_AUTO);

    // сигнал потоку завершиться
    Param.booStop = -1;
    // справоцировать выход потока из sem_wait()
    sem_post (&Param.semaRing);

    // дождаться остановки потока   
    pthread_join (thread, NULL);

    // закрытие ресурсов ...
    CloseRingBuffer (&Param.Ring);
    ev_loop_destroy (loop);
    sem_destroy (&Param.semaRing);
    close (sock);

    return 0;
}
