#include "../include/conexiones.h"

// LEVANTAR CLIENTE
int levantarCliente(t_log *logger, char *nombre_servidor, char *ip, char *puerto)
{
    int socket_cliente = crear_conexion(logger, nombre_servidor, ip, puerto);
    if (socket_cliente == -1)
    {
        log_error(logger, "No se pudo conectar al servidor %s:%s", ip, nombre_servidor);
        return -1;
    }
    return socket_cliente;
}
