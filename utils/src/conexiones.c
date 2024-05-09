#include "../include/conexiones.h"

// LEVANTAR SERVIDOR
void levantarServidor(t_log *logger, char *puerto, char *ip, char *nombreServidor)
{
    int server_fd = iniciar_servidor(logger, nombreServidor, ip, puerto);
    log_info(logger, "Servidor: %s listo para recibir al cliente", nombreServidor);
    int cliente_fd = esperar_cliente(logger, nombreServidor, server_fd);

    t_list *lista;
    while (1)
    {
        int cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case MENSAJE:
            recibir_mensaje(cliente_fd);
            break;
        case PAQUETE:
            lista = recibir_paquete(cliente_fd);
            log_info(logger, "Me llegaron los siguientes valores:\n");
            list_iterate(lista, (void *)iterator);
            break;
        case -1:
            log_error(logger, "el cliente se desconecto. Terminando servidor");
            return EXIT_FAILURE;
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            break;
        }
    }
    return EXIT_SUCCESS;
}
void iterator(char *value)
{
    log_info(logger, "%s", value);
}

// LEVANTAR CLIENTE
int levantarCliente(t_log *logger, char *nombreDelServidor, char *ip, char *puerto, char *mensaje)
{
    int unaConexion = crear_conexion(logger, nombreDelServidor, ip, puerto);
    // enviar_mensaje(mensaje, unaConexion);
    return unaConexion;
}
