#include "../include/sockets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// INICIA SERVER ESCUCHANDO EN IP:PUERTO
int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto)
{
    int socket_servidor;
    struct addrinfo hints, *servinfo;

    // Inicializando hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe los addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    bool conecto = false;

    // Itera por cada addrinfo devuelto
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next)
    {
        socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (socket_servidor == -1) // fallo de crear socket
            continue;

        if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
            perror("setsockopt(SO_REUSEADDR) failed");

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1)
        {
            // Si entra aca fallo el bind
            close(socket_servidor);
            continue;
        }
        // Ni bien conecta uno nos vamos del for
        conecto = true;
        break;
    }

    if (!conecto)
    {
        free(servinfo);
        return 0;
    }

    listen(socket_servidor, SOMAXCONN); // Escuchando (hasta SOMAXCONN conexiones simultaneas)

    // Aviso al logger
    log_info(logger, "Escuchando en %s:%s (%s)\n", ip, puerto, name);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int crear_servidor(int puerto)
{
    int socket_servidor;

    const char *puerto_recibido = string_itoa(puerto);

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, puerto_recibido, &hints, &servinfo) != 0)
    {
        perror("getaddrinfo");
        free(puerto_recibido);
        exit(EXIT_FAILURE);
    }

    printf("Servidor - Puerto <%d>\n", puerto);

    socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (socket_servidor == -1)
    {
        perror("socket");
        freeaddrinfo(servinfo);
        free(puerto_recibido);
        exit(EXIT_FAILURE);
    }

    int val = 1;
    if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
    {
        perror("setsockopt");
        close(socket_servidor);
        freeaddrinfo(servinfo);
        free(puerto_recibido);
        exit(EXIT_FAILURE);
    }

    if (bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        perror("bind");
        close(socket_servidor);
        freeaddrinfo(servinfo);
        free(puerto_recibido);
        exit(EXIT_FAILURE);
    }

    if (listen(socket_servidor, SOMAXCONN) == -1)
    {
        perror("listen");
        close(socket_servidor);
        freeaddrinfo(servinfo);
        free(puerto_recibido);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);
    free(puerto_recibido);

    return socket_servidor;
}

// ESPERAR CONEXION DE CLIENTE EN UN SERVER ABIERTO
int esperar_cliente(t_log *logger, const char *name_server, const char *name_client, int socket_servidor)
{
    struct sockaddr_storage cliente_addr;
    socklen_t addr_size = sizeof(cliente_addr);

    int socket_cliente = accept(socket_servidor, (struct sockaddr *)&cliente_addr, &addr_size);
    if (socket_cliente == -1)
    {
        perror("accept failed");
        return -1;
    }
    printf("<%s> - Conectado A: <%s>\n", name_client, name_server);
    return socket_cliente;
}

// CLIENTE SE INTENTA CONECTAR A SERVER ESCUCHANDO EN IP:PUERTO
int crear_conexion(t_log *logger, const char *server_name, char *ip, char *puerto)
{
    struct addrinfo hints, *servinfo;

    // Init de hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    // Crea un socket con la informacion recibida (del primero, suficiente)
    int socket_cliente = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    int val = 1;
    setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    if (connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        return -1;
    }

    freeaddrinfo(servinfo);

    return socket_cliente;
}

// CERRAR CONEXION
void liberar_conexion(int *socket_cliente)
{
    close(*socket_cliente);
    *socket_cliente = -1;
}

char *obtener_ip_local()
{
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char *ip = malloc(INET_ADDRSTRLEN);

    if (ip == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        free(ip);
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET)
        { // IPv4
            if (strcmp(ifa->ifa_name, "lo") != 0)
            { // Ignorar loopback
                if (inet_ntop(family, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip, INET_ADDRSTRLEN) != NULL)
                {
                    freeifaddrs(ifaddr);
                    printf("Direccion de memoria de la IP local: %p\n", ip);
                    return ip;
                }
                else
                {
                    perror("inet_ntop");
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    free(ip);
    return NULL;
}
