#include "../include/conexiones.h"

// LEVANTAR CLIENTE
int levantarCliente(t_log *logger, char *nombreDelServidor, char *ip, char *puerto)
{
    printf_blue("Intentando conectar cpu con memoria, IP memoria: %s PUERTO memoria: %s", ip, puerto);

    int unaConexion = crear_conexion(logger, nombreDelServidor, ip, puerto);
    return unaConexion;
}
