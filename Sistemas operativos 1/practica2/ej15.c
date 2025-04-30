#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <netinet/in.h>
#include <netinet/ip.h>

#define N_THREADS 8
#define MAX_EVENTS 3

int epollfd;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int U = 0;

void quit(char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

int fd_readline(int fd, char *buf)
{
  int rc;
  int i = 0;

  while ((rc = read(fd, buf + i, 1)) > 0)
  {
    if (buf[i] == '\n')
      break;
    i++;
  }

  if (rc < 0)
    return rc;

  buf[i] = 0;
  return i;
}

void handle_conn(int csock)
{
  char buf[200];
  int rc;

  while (1)
  {
    /* Atendemos pedidos, uno por linea */
    rc = fd_readline(csock, buf);
    if (rc < 0)
      quit("read... raro");

    if (rc == 0)
    {
      /* linea vacia, se cerró la conexión */
      close(csock);
      return;
    }

    struct epoll_event ev;
    if (!strcmp(buf, "NUEVO"))
    {
      char reply[20];
      pthread_mutex_lock(&mutex);
      sprintf(reply, "%d\n", U);
      U++;
      pthread_mutex_unlock(&mutex);
      write(csock, reply, strlen(reply));
      /*       ev.events = EPOLLIN | EPOLLONESHOT;
            ev.data.fd = csock;
            if (epoll_ctl(epollfd, EPOLL_CTL_MOD, csock, &ev) == -1)
            {
              perror("epoll_ctl: conn_sock");
              exit(EXIT_FAILURE);
            } */
    }
    else if (!strcmp(buf, "CHAU"))
    {
      write(csock, "Chau!, cierro la conexion.\n", 26);
      /*       if (epoll_ctl(epollfd, EPOLL_CTL_DEL, csock, &ev) == -1)
            {
              perror("epoll_ctl: conn_sock");
              exit(EXIT_FAILURE);
            } */
      close(csock);
    }
  }
  return;
}

void wait_for_clients(struct epoll_event *ev_listen)
{
  struct epoll_event events[MAX_EVENTS], ev;
  int nfds;
  int listen_sock = ev_listen->data.fd, conn_sock;

  for (;;)
  {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (nfds == -1)
    {
      perror("epoll wait");
      exit(EXIT_FAILURE);
    }

    for (int n = 0; n < nfds; ++n)
    {
      if (events[n].data.fd == listen_sock)
      {
        conn_sock = accept(listen_sock, NULL, NULL);
        if (conn_sock == -1)
        {
          perror("accept");
          exit(EXIT_FAILURE);
        }
        ev.events = EPOLLIN | EPOLLONESHOT;
        ev.data.fd = conn_sock;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
        {
          perror("epoll_ctl: conn_sock.");
          exit(EXIT_FAILURE);
        }

        ev.events = EPOLLIN | EPOLLONESHOT;
        ev.data.fd = listen_sock;
        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, listen_sock, &ev) == -1)
        {
          perror("epoll_ctl: listen_sock.");
          exit(EXIT_FAILURE);
        }
      }
      else
      {
        handle_conn(events[n].data.fd);
      }
    }
  }
}

int mk_lsock()
{
  struct sockaddr_in sa;
  int lsock;
  int rc;
  int yes = 1;

  lsock = socket(AF_INET, SOCK_STREAM, 0);
  if (lsock < 0)
    quit("socket");

  if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == 1)
    quit("setsockopt");

  sa.sin_family = AF_INET;
  sa.sin_port = htons(4040);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  rc = bind(lsock, (struct sockaddr *)&sa, sizeof sa);
  if (rc < 0)
    quit("bind");

  rc = listen(lsock, 10);
  if (rc < 0)
    quit("listen");

  return lsock;
}

int main()
{
  int lsock = mk_lsock();

  epollfd = epoll_create1(0);
  if (epollfd == -1)
  {
    perror("Epoll_create1 error.");
    exit(EXIT_FAILURE);
  }

  struct epoll_event ev;

  ev.events = EPOLLIN | EPOLLONESHOT;
  ev.data.fd = lsock;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lsock, &ev) == -1)
  {
    perror("epoll_ctl: lsock.");
    exit(EXIT_FAILURE);
  }

  pthread_t threads[N_THREADS];
  for (int i = 0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, wait_for_clients, &ev);

  for (int i = 0; i < N_THREADS; i++)
    pthread_join(threads[i], NULL);

  return 0;
}
