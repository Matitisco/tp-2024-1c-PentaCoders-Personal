#include "../include/serializacion.h"


t_log *logger;



/*----------------------------------- CODIGO -----------------------------------*/

// ENVIAR CODIGO
void enviar_cod_enum(int socket_servidor, uint32_t cod)
{
    send(socket_servidor, &cod, sizeof(uint32_t), 0);
}
// RECIBIR CODIGO
uint32_t recibir_cod(int socket_cliente)
{
    uint32_t codigo;
    recv(socket_cliente, &codigo, sizeof(uint32_t), MSG_WAITALL);
    return codigo;
}

/*----------------------------------- BUFFER -----------------------------------*/

// CREAR BUFFER
tipo_buffer *crear_buffer()
{
    tipo_buffer *unBuffer = malloc(sizeof(unBuffer));
    if (unBuffer == NULL)
    {
        return NULL;
    }
    else
    {
        unBuffer->size = 0; // inicializo los valores para que no haya cosas rawras
        unBuffer->offset = 0;
        unBuffer->stream = NULL;
        return unBuffer; // lo retorno
    }
}
// DESTRUIR BUFFER
void destruir_buffer(tipo_buffer *unBuffer)
{
    if (unBuffer != NULL)
    {                           // esto es mas que nada para chequear
        free(unBuffer->stream); // primero liberamos al puntero
        free(unBuffer);         // lugeo loberamos al buffer
    }
}
// ESCRIBIR EN EL BUFFER
void escribir_buffer(tipo_buffer *buffer, uint8_t entero)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
    if (buffer->stream == NULL)
    {
        // Por si hay algun error
        return;
    }
    memcpy((char *)buffer->stream + buffer->size, &entero, sizeof(uint8_t));
    buffer->size += sizeof(uint8_t);
}
// ENVIAR BUFFER
void enviar_buffer(tipo_buffer *buffer, int socket)
{
    // Enviamos el tamanio del buffer
    send(socket, &(buffer->size), sizeof(uint32_t), 0);

    if (buffer->size != 0)
    {
        // Enviamos el stream del buffer
        send(socket, buffer->stream, buffer->size, 0);
    }
}
// RECIBIR BUFFER

// LEER BUFFER
uint32_t leer_buffer_entero(tipo_buffer *buffer)
{
    uint32_t entero;

    memcpy(&entero, buffer->stream + buffer->offset, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);

    return entero;
}

t_instruccion *leer_buffer_instruccion(tipo_buffer *buffer) // falta poner las de 4 parametros
{
    t_instruccion *instruccion = malloc(sizeof(t_instruccion));
    instruccion->parametros = list_create();
    uint8_t codigo;

    instruccion->codigo = codigo = leer_buffer_entero(buffer);

    switch (codigo)
    {
    case SET:
    case MOV_IN:
    case MOV_OUT:
    case SUM:
    case SUB:
    case JNZ:
    case RESIZE:
    case COPY_STRING:
    case SIGNAL:
    case IO_GEN_SLEEP:
        paramBufferALista(instruccion, 2, buffer);
        break;
    case IO_STDIN_READ:
    case IO_STDOUT_WRITE:
    case IO_FS_CREATE:
    case IO_FS_DELETE:
    case IO_FS_WRITE:
    case IO_FS_TRUNCATE:
        paramBufferALista(instruccion, 3, buffer);
        break;
    case IO_FS_READ:
        paramBufferALista(instruccion, 5, buffer);
        break;
    case EXIT:
        paramBufferALista(instruccion, 0, buffer);
        break;
    default:
        log_info(logger, "No se reconoce la instrucción");
        break;
    }

    return instruccion;
}

void paramBufferALista(t_instruccion *instruccion, int cantParam, tipo_buffer *buffer)
{
    char *parametro;
    size_t* size=malloc(sizeof(size_t));

    for (int i = 0; i < cantParam; i++)
    {
        parametro = leer_buffer_string(buffer, &size);
        list_add(instruccion->parametros, parametro);
    }
}
char *leer_buffer_string(tipo_buffer *buffer, uint32_t *tam)
{
    (*tam) = leer_buffer_entero(buffer);
    char *cadena = malloc((*tam) + 1);

    memcpy(cadena, buffer->stream + buffer->offset, (*tam));
    buffer->offset += (*tam);

    *(cadena + (*tam)) = '\0';

    return cadena;
}

/*
t_registros *leer_buffer_registros(tipo_buffer *buffer)
{
    t_registros *reg = malloc(sizeof(t_registros));

    reg->AX = leer_buffer_entero(buffer);
    reg->BX = leer_buffer_entero(buffer);
    reg->CX = leer_buffer_entero(buffer);
    reg->DX = leer_buffer_entero(buffer);
    reg->EAX = leer_buffer_entero(buffer);
    reg->EBX = leer_buffer_entero(buffer);
    reg->ECX = leer_buffer_entero(buffer);
    reg->EDX = leer_buffer_entero(buffer);
    reg->SI = leer_buffer_entero(buffer);
    reg->DI = leer_buffer_entero(buffer);

    return reg;
}


void *leer_buffer_pagina(tipo_buffer *buffer, uint32_t tamPagina) // PARA MEMORIA
{
    void *paginaLeida = malloc(tamPagina);

    memcpy(paginaLeida, buffer->stream + buffer->offset, tamPagina);
    buffer->offset += tamPagina;

    for (int i = 0; i < (tamPagina / 4); i++)
    {
        uint32_t k;
        memcpy(&k, paginaLeida + i * 4, sizeof(uint32_t));
    }

    return paginaLeida;
}
*/