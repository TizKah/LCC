#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <unistd.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>

#include "tablahash.h"

#define SOCKET_PORT 3942
#define BACKLOG 10
#define EPOLL_MAX_EVENTS 20

#define THREADS_N 4
#define STORAGE_SIZE 128

struct _key_value_pair
{
  char key[16];
  char value[128];
};
typedef struct _key_value_pair kvp;

/*
  Implemente un servidor que provea un key-value store a sus clientes.
  El servidor debe esperar conexiones en el puerto 3942 TCP (AF_INET, SOCK_STREAM) y atender los pedidos
  de cada cliente. Un pedido es siempre una secuencia de palabras separadas por espacios, terminado por
  un caracter de nueva lınea (’\n’). La primer palabra es el comando y el resto (alguna cantidad) son los
  argumentos al comando. Ninguna palabra contiene espacios ni caracteres no alfanumericos. Los pedidos
  posibles son:

  - PUT k v: introduce al store el valor v bajo la clave k. El valor viejo para k, si existıa, es pisock_addrdo. El
  servidor debe responder con OK.

  - DEL k: Borra el valor asociado a la clave k. El servidor debe responder con OK.

  - GET k: Busca el valor asociado a la clave k. El servidor debe contestar con OK v si el valor es v, o
  con NOTFOUND si no hay valor asociado a k.

  Ante cualquier otro mensock_addrje el servidor responde con EINVAL. Las respuestas del servidor siempre terminan
  con ’\n’. (El servidor puede probarse facilmente con netcat.) Por supuesto, deben soportarse conexiones
  simultaneas de varios clientes. Ver tambien:

  Beejs’ Guide to Network Programming - https://beej.us/guide/bgnet/html/
  Man pages: man 2 socket, man 2 bind, man 2 listen, man 2 accept
*/

int epoll_fd;
TablaHash hash_table;
pthread_mutex_t hash_mutex = PTHREAD_MUTEX_INITIALIZER;

kvp *kvp_create(char *key, char *value)
{
  kvp *new_kvp = malloc(sizeof(kvp));
  strcpy(new_kvp->key, key);
  strcpy(new_kvp->value, value);
  return new_kvp;
}

int kvp_comp(void *data1, void *data2)
{
  kvp *kvp1 = (kvp *)data1, *kvp2 = (kvp *)data2;
  return strcmp(kvp1->key, kvp2->key);
}

void *kvp_copy(void *data)
{
  kvp *kvp_copy = malloc(sizeof(kvp));
  strcpy(kvp_copy->key, ((kvp *)data)->key);
  strcpy(kvp_copy->value, ((kvp *)data)->value);
  return (void *)kvp_copy;
}

void kvp_destroy(void *data)
{
  free(data);
}

unsigned kvp_hash(void *data)
{
  kvp *kvp0 = (kvp *)data;
  char *str = kvp0->key;
  unsigned long hash = 5381;

  int c;
  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

int err_exit(char *calling_fn, char *error)
{
  printf("%s err: %s\n", calling_fn, error);
  exit(EXIT_FAILURE);
}

int execute_command(int csock, char *command)
{
  if (command[3] != ' ')
  {
    printf("bad command\n");
    return 1;
  }

  char *arg0, *arg1;
  arg0 = strchr(command, ' ') + 1;
  arg1 = strchr(arg0, ' ') + 1;

  int arg0_len = strlen(arg0);
  for (int i = 0; i < arg0_len; i++)
    if (arg0[i] == ' ')
      arg0[i] = '\0';

  command[3] = '\0';

  if (strcmp(command, "PUT") == 0)
  {
    if (arg1 == NULL)
      write(csock, "PUT MISSING VALUE", sizeof("PUT MISSING VALUE"));
    else
    {
      pthread_mutex_lock(&hash_mutex);
      tablahash_insertar(hash_table, kvp_create(arg0, arg1));
      pthread_mutex_unlock(&hash_mutex);
      write(csock, "OK", sizeof("OK"));
    }
  }
  else if (strcmp(command, "DEL") == 0)
  {
    kvp kvp0;
    strcpy(kvp0.key, arg0);
    pthread_mutex_lock(&hash_mutex);
    int can_delete = tablahash_eliminar(hash_table, (void *)&kvp0);
    pthread_mutex_unlock(&hash_mutex);
    if (can_delete)
      write(csock, "OK", sizeof("OK"));
    else
      write(csock, "NOTFOUND", sizeof("NOTFOUND"));
  }
  else if (strcmp(command, "GET") == 0)
  {
    kvp kvp0;
    strcpy(kvp0.key, arg0);
    pthread_mutex_lock(&hash_mutex);
    kvp *kvp_to_find = (kvp *)tablahash_buscar(hash_table, (void *)&kvp0);
    pthread_mutex_unlock(&hash_mutex);
    if (kvp_to_find == NULL)
    {
      puts("not found");
      write(csock, "NOTFOUND", sizeof("NOTFOUND"));
    }
    else
    {
      puts("found");
      write(csock, "OK ", sizeof("OK "));
      write(csock, kvp_to_find->value, strlen(kvp_to_find->value));
    }
  }
  else
    write(csock, "EINVAL", sizeof("EINVAL"));
  write(csock, "\n", sizeof("\n"));
  return 0;
}

int fd_readline(int fd, char *buffer)
{
  int read_count;
  int i = 0;

  while ((read_count = read(fd, buffer + i, 1)) > 0)
  {
    if (buffer[i] == '\n')
      break;
    i++;
  }

  if (read_count < 0)
    return read_count;

  buffer[i] = 0;
  return i;
}

void handle_conn(int csock)
{
  printf("handling\n");
  char buffer[1024];
  int read_count;

  for (;;)
  {
    /* Atendemos pedidos, uno por linea */
    read_count = fd_readline(csock, buffer);
    if (read_count < 0)
      err_exit("fd_readline", "wrong read");
    printf("read: %s\n", buffer);

    /* Linea vacia, cerrar conexion */
    if (read_count == 0)
    {
      close(csock);
      return;
    }

    execute_command(csock, buffer);
  }
  return;
}

void *wait_for_clients(struct epoll_event *ev_listen)
{
  printf("start\n");

  struct epoll_event events[EPOLL_MAX_EVENTS], ev;
  int n_fds, listen_sock = ev_listen->data.fd, conn_sock;

  for (;;)
  {
    n_fds = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, -1);
    if (n_fds == -1)
      err_exit("wait_for_clients", "epoll_wait");

    for (int n = 0; n < n_fds; n++)
    {
      /* Si la conexion es nueva la aceptamos */
      if (events[n].data.fd == listen_sock)
      {
        conn_sock = accept(listen_sock, NULL, NULL);
        if (conn_sock == -1)
          err_exit("wait_for_clients", "accept");
        printf("accepted\n");

        ev.events = EPOLLIN | EPOLLONESHOT;
        ev.data.fd = conn_sock;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
          err_exit("wait_for_clients", "epoll_ctl_add");

        ev.events = EPOLLIN | EPOLLONESHOT;
        ev.data.fd = listen_sock;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, listen_sock, &ev) == -1)
          err_exit("wait_for_clients", "epoll_ctl_mod");

        write(conn_sock, "ACCEPTED\n", sizeof("ACCEPTED\n"));
      }
      else
      {
        handle_conn(events[n].data.fd);
      }
    }
  }

  return NULL;
}

int get_socket()
{
  /* Creamos la socket address en el puerto 3942 */
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // IPv4 o IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

  /* Converting port to string */
  char sock_port_str[(int)((ceil(log10(SOCKET_PORT)) + 1) * sizeof(char))];
  sprintf(sock_port_str, "%d", SOCKET_PORT);
  getaddrinfo(NULL, sock_port_str, &hints, &res);

  /* Creamos el socket */
  int msocket = socket(AF_INET, SOCK_STREAM, 0);
  if (msocket < 0)
    err_exit("get_socket", "socket");

  /* Declaramos el socket como reutilziable para evitar errores */
  int yes = 1;
  if (setsockopt(msocket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &yes, sizeof yes) == 1)
    err_exit("get_socket", "setsockopt");

  int rc;

  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(SOCKET_PORT);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  rc = bind(msocket, (struct sockaddr *)&sa, sizeof sa);

  /* Bindear al puerto 3942 TCP, en todas las direcciones disponibles */
  // rc = bind(msocket, (struct sockaddr *)res->ai_socktype, res->ai_protocol);
  if (rc < 0)
    err_exit("get_socket", "bind");

  /* Setear en modo escucha */
  rc = listen(msocket, BACKLOG);
  if (rc < 0)
    err_exit("get_socket", "listen");

  return msocket;
}

int server()
{
  hash_table = tablahash_crear(STORAGE_SIZE, kvp_copy, kvp_comp, kvp_destroy, kvp_hash);
  assert(hash_table != NULL);

  int listen_socket = get_socket();

  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1)
    err_exit("server", "epoll_create");

  struct epoll_event ev;

  ev.events = EPOLLIN | EPOLLONESHOT;
  ev.data.fd = listen_socket;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &ev) < 0)
    err_exit("server", "epoll_ctl");

  printf("creating threads\n");
  pthread_t threads[THREADS_N];
  for (int i = 0; i < THREADS_N; i++)
    pthread_create(&threads[i], NULL, wait_for_clients, &ev);

  for (int i = 0; i < THREADS_N; i++)
    pthread_join(threads[i], NULL);

  return 0;
}

int main()
{
  return server();
}