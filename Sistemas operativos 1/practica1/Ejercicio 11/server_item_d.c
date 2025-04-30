#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/select.h>

/*
 * Para probar, usar netcat. Ej:
 *
 *      $ nc localhost 4040
 *      NUEVO
 *      0
 *      NUEVO
 *      1
 *      CHAU
 */

void quit(char *s)
{
    perror(s);
    abort();
}

int fd_readline(int fd, char *buf)
{
    int rc;
    int i = 0;

    /*
     * Leemos de a un caracter (no muy eficiente...) hasta
     * completar una línea.
     */
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

void handle_conn(int csock, int *u, fd_set *readfds)
{
    char buf[200];
    int rc;

    /* Atendemos pedidos, uno por linea */
    rc = fd_readline(csock, buf);
    if (rc < 0)
        quit("read... raro");

    if (rc == 0)
    {
        /* linea vacia, se cerró la conexión */
        close(csock);
        FD_CLR(csock, readfds); // Removemos el descriptor de archivo de readfds
        return;
    }

    if (!strcmp(buf, "NUEVO"))
    {
        char reply[20];
        sprintf(reply, "%d\n", *u);
        (*u)++;
        write(csock, reply, strlen(reply));
    }
    else if (!strcmp(buf, "CHAU"))
    {
        close(csock);
        FD_CLR(csock, readfds); // Removemos el descriptor de archivo de readfds
        return;
    }
}

void wait_for_clients(int lsock, int *u)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(lsock, &readfds); // Agregamos el descriptor de archivo del socket de escucha

    int max_fd = lsock;
    while (1)
    {
        fd_set tmp_fds = readfds;

        // Usamos select para esperar eventos en los descriptores de archivo
        if (select(max_fd + 1, &tmp_fds, NULL, NULL, NULL) < 0)
            quit("select");

        // Verificamos cada descriptor de archivo para determinar si hay datos disponibles
        for (int i = 0; i <= max_fd; ++i)
        {
            if (FD_ISSET(i, &tmp_fds))
            {
                if (i == lsock)
                {
                    // Nueva conexión entrante
                    int csock = accept(lsock, NULL, NULL);
                    if (csock < 0)
                        quit("accept");
                    FD_SET(csock, &readfds); // Agregamos el nuevo descriptor de archivo al conjunto
                    if (csock > max_fd)
                        max_fd = csock; // Actualizamos el máximo descriptor de archivo si es necesario
                }
                else
                {
                    // Datos disponibles en un socket cliente existente
                    handle_conn(i, u, &readfds);
                }
            }
        }
    }
}

/* Crea un socket de escucha en puerto 4040 TCP */
int mk_lsock()
{
    struct sockaddr_in sa;
    int lsock;
    int rc;
    int yes = 1;

    /* Crear socket */
    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0)
        quit("socket");

    /* Setear opción reuseaddr... normalmente no es necesario */
    if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == 1)
        quit("setsockopt");

    sa.sin_family = AF_INET;
    sa.sin_port = htons(4040);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Bindear al puerto 4040 TCP, en todas las direcciones disponibles */
    rc = bind(lsock, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0)
        quit("bind");

    /* Setear en modo escucha */
    rc = listen(lsock, 10);
    if (rc < 0)
        quit("listen");

    return lsock;
}

int main()
{
    int U_counter = 0;
    int lsock;
    lsock = mk_lsock();
    wait_for_clients(lsock, &U_counter);
    return 0;
}
