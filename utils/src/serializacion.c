#include "../include/serializacion.h"
t_log *logger;

/*----------------------------------- CODIGO -----------------------------------*/

// ENVIAR CODIGO
void enviar_cod_enum(int socket_servidor, uint32_t cod)
{
    send(socket_servidor, &cod, sizeof(uint32_t), 0);
    printf("Se envio el codigo al servidor %u", cod);
}

// RECIBIR CODIGO
uint32_t recibir_cod(int socket_cliente)
{
    uint32_t codigo;
    recv(socket_cliente, &codigo, sizeof(uint32_t), MSG_WAITALL);
    return codigo;
}

int recibir_operacion(int socket_cliente)
{
    int cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) > 0)
        return cod_op;
    else
    {
        close(socket_cliente);
        return -1;
    }
}

/*----------------------------------- BUFFER -----------------------------------*/

// CREAR BUFFER
/*
void crear_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(tipo_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}*/

// DESTRUIR BUFFER

// ESCRIBIR EN EL BUFFER

// ENVIAR BUFFER

// RECIBIR BUFFER
// lo reemplazamos por el nuestro
/* void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
} */
// LEER BUFFER
