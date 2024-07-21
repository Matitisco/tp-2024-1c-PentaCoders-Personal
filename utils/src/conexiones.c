#include "../include/conexiones.h"

// LEVANTAR CLIENTE
int levantarCliente(t_log *logger, char *nombreDelServidor, char *ip, char *puerto)
{
    int unaConexion = crear_conexion(logger, nombreDelServidor, ip, puerto);
    return unaConexion;
}
