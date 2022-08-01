#include "my_2.h"

static void sender_to_client_cb (struct ev_loop *evLoop, ev_io *w, int ) {
    char acBuffer [1024];
    _Prm * prm = ev_userdata (evLoop);

    struct sockaddr_un  clnt_addr;
    int caddrlen;
    caddrlen = sizeof(clnt_addr);

    int n = recvfrom (w->fd, acBuffer, sizeof(acBuffer), 0,
        (struct sockaddr*)&clnt_addr, &caddrlen);
    if (n > 0) {
        acBuffer [n] = 0;
printf ("2> %s\n");
        send (prm->iSock, acBuffer, n, 0);
    }
}

static void recv_cb (struct ev_loop *evLoop, ev_io *w, int ) {

    char acBuffer [1024];
    int ret = recv (w->fd,acBuffer,sizeof(acBuffer)-1,0);
    if (ret > 0) {
        _Prm * prm = ev_userdata (evLoop);
        acBuffer [ret] = 0;
        printf ("2< %s", acBuffer);

        // послать строку первому потоку
        sendto (prm->iExchange, acBuffer, strlen(acBuffer), 0,
            (struct sockaddr*)&prm->unServerAddress,sizeof(prm->unServerAddress)
        );
    } else if ((ret < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        ;
    } else {
        puts ("remote socket closed!");
        // отменить обработку
        ev_break (evLoop, EVBREAK_ALL);
    }

}

void* thread_2 (void* args) {
    puts ("new client ...");

    _Prm *prm = malloc(sizeof(*prm));
    if (!prm) {
        puts ("-ERROR small memory");
        return NULL;
    }

    memcpy (prm, args, sizeof(*prm));
    printf ("iSock= %i\n", prm->iSock);
    struct sockaddr_un unClientAddress; // только лишь для удаления временного файла
    prm->iExchange = GetUnixSock (prm->unServerAddress.sun_path, &unClientAddress);

    if (prm->iExchange >= 0) {
        struct ev_loop *evloop_2 = ev_loop_new (EVFLAG_AUTO);
        if (evloop_2) {
            ev_io w_Client;
            ev_io_init (&w_Client, recv_cb, prm->iSock, EV_READ);
            ev_io_start (evloop_2, &w_Client); 
            // добавить обработчик для обмена с первым потоком
            ev_io w_Exchange;
            ev_io_init (&w_Exchange, sender_to_client_cb, prm->iExchange, EV_READ);
            ev_io_start (evloop_2, &w_Exchange);
            
            ev_set_userdata (evloop_2, prm);
            
            ev_loop (evloop_2,0);
            
            ev_loop_destroy (evloop_2);
        } else
            puts ("-ERROR small memory evlib");

        close (prm->iExchange);
        unlink (unClientAddress.sun_path);
    } else
        puts ("-ERROR no local sockets");

    close (prm->iSock);
    free (prm);

puts ("thread ENDED");
    return NULL;
}

