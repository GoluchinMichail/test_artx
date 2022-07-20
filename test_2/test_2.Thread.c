#include "my_2.h"

static _ParamThread *prm;

static void recv_callback (struct ev_loop *evLoop, ev_io *w, int );

static void write_callback (struct ev_loop *evLoop, ev_io *w, int ) {
    write (w->fd,prm->acBuffer,strlen(prm->acBuffer));

    ev_io_stop (evLoop,  w);
    ev_io_init (w,recv_callback,w->fd,EV_READ);
    ev_io_start (evLoop,w);
}

static void PostMessage (void) {

    // "первый" свободен ?
    if (ev_async_pending(&prm->watcher_1)==0) {
        if (memcmp(prm->acBuffer,"+++",3))
            // не завершение, предполагается обработка acBuffer
            pthread_mutex_lock (&prm->LockBuffer);  // пусть 1-й поток и разблокирует это
        ev_async_send (prm->loop_1, &prm->watcher_1);
    }
}

static void recv_callback (struct ev_loop *evLoop, ev_io *w, int ) {

    int ret = recv (w->fd,prm->acBuffer,sizeof(prm->acBuffer),0);
    if (ret > 0) {
        prm->acBuffer[ret] = 0;
        printf (prm->acBuffer);

        // сообщить 1-му потоку о доступности буфера
        PostMessage ();

        // дождаться обработки строки в 1-м потоке
//    puts ("SEND REPONSE ...");
//    если что-то печатать (как выше) - будет достаточная пауза для отработки 1-го потока
//    и далее посылается перевёрнутая строка :)  мутексы не нужны :))
        pthread_mutex_lock (&prm->LockBuffer); // пусть 1-й поток разблокирует это
        pthread_mutex_unlock (&prm->LockBuffer);

        if (!memcmp(prm->acBuffer,"+++",3)) {
        // завершение ...
            ev_io_stop (evLoop, w);
            ev_io_init (w,NULL,0,0);// как ещё остановить это - итио ^_^     извините :)
        } else {
            // послать ответ ... тот что в  prm->acBuffer
            ev_io_stop (evLoop,  w);
            ev_io_init (w, write_callback, w->fd, EV_WRITE);
            ev_io_start (evLoop, w);
        }

    } else {
        puts ("remote socket closed!");

        ev_io_stop (evLoop, w);
        ev_io_init (w,NULL,0,0);// как ещё остановить это - итио ^_^     извините :)

        // сообщить основному потоку о завершении
        strcpy (prm->acBuffer,"+++");
        PostMessage ();
    }

}

static void accept_callback (struct ev_loop *evLoop, ev_io *w, int ) {
    int newfd;
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(struct sockaddr);

    while ((newfd = accept(w->fd, (struct sockaddr *)&sin, &addrlen)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue; 
        } else {
            printf ("accept error.[%s]\n", strerror(errno));
            break;
        }
    }

    ev_io_stop (evLoop,w);
    ev_io_init (w,recv_callback,newfd,EV_READ);
    ev_io_start (evLoop,w);

    printf("accept callback : fd :%d\n",w->fd);
}

void * thread_2 (void* args) {
    puts ("THREAD_2 >>>>>>>>>>>>>>>>>>>>>>>");

    prm = args;

    struct ev_loop *evloop_2 = ev_loop_new (EVFLAG_AUTO);
    ev_io ev_io_watcher_2;
    ev_io_init (&ev_io_watcher_2, accept_callback, prm->iSocket, EV_READ);
    ev_io_start (evloop_2, &ev_io_watcher_2); 
    ev_loop (evloop_2,0);

    ev_loop_destroy (evloop_2);

    puts ("THREAD_2 <<<<<<<<<<<<<<<<<<<<<<<");
    return NULL;
}
