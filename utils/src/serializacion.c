#include "../include/serializacion.h"
t_log *logger;

void enviar_cod_enum(int socket_servidor, uint32_t cod)
{
    send(socket_servidor, &cod, sizeof(uint32_t), 0);
}
op_code recibir_operacion(int socket_cliente)
{
    op_code cod_op;
    while (1)
    {
        if (recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) > 0)
            return cod_op;
        else
        {
            close(socket_cliente);
            return -1;
        }
    }
}

tipo_buffer *crear_buffer()
{
    tipo_buffer *buffer = malloc(sizeof(tipo_buffer));
    buffer->offset = 0;
    buffer->size = 0;
    buffer->stream = NULL;
    return buffer;
}

void destruir_buffer(tipo_buffer *buffer)
{
    free(buffer->stream);
    free(buffer);
}

void agregar_buffer_para_enterosUint32(tipo_buffer *buffer, uint32_t entero)
{

    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint32_t));
    buffer->size += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
}

void agregar_buffer_para_enterosUint8(tipo_buffer *buffer, uint8_t entero)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
    buffer->size += sizeof(uint8_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
}

void agregar_buffer_para_string(tipo_buffer *buffer, char *args)
{
    uint32_t tamanio = 0;

    char *string = (char *)args;

    if (args == NULL)
    {
        return;
    }

    while (string[tamanio] != NULL)
    {
        tamanio++;
    }
    agregar_buffer_para_enterosUint32(buffer, tamanio);

    buffer->stream = realloc(buffer->stream, buffer->size + tamanio);
    memcpy(buffer->stream + buffer->size, string, tamanio);
    buffer->size += tamanio;
}

void agregar_buffer_para_registros(tipo_buffer *buffer, t_registros *registros)
{
    agregar_buffer_para_enterosUint8(buffer, registros->AX);
    agregar_buffer_para_enterosUint8(buffer, registros->BX);
    agregar_buffer_para_enterosUint8(buffer, registros->CX);
    agregar_buffer_para_enterosUint8(buffer, registros->DX);
    agregar_buffer_para_enterosUint32(buffer, registros->EAX);
    agregar_buffer_para_enterosUint32(buffer, registros->EBX);
    agregar_buffer_para_enterosUint32(buffer, registros->ECX);
    agregar_buffer_para_enterosUint32(buffer, registros->EDX);
    agregar_buffer_para_enterosUint32(buffer, registros->SI);
    agregar_buffer_para_enterosUint32(buffer, registros->DI);
}

void enviar_buffer(tipo_buffer *buffer, int socket)
{
    send(socket, &(buffer->size), sizeof(uint32_t), 0); // enviar size

    if (buffer->size != 0)
    {
        // Enviamos el stream del buffer
        send(socket, buffer->stream, buffer->size, 0); // enviar buffer
    }
    else
    {
        printf("El buffer tiene size 0");
    }
}

tipo_buffer *recibir_buffer(int socket)
{
    tipo_buffer *buffer = crear_buffer();
    // Recibo el tamanio del buffer y reservo espacio en memoria
    recv(socket, &(buffer->size), sizeof(uint32_t), MSG_WAITALL);
    if (buffer->size != 0)
    {
        buffer->stream = malloc(buffer->size);
        // Recibo stream del buffer
        recv(socket, buffer->stream, buffer->size, MSG_WAITALL);
    }
    return buffer;
}

uint32_t leer_buffer_enteroUint32(tipo_buffer *buffer)
{
    int entero32; // malloc(sizeof(uint32_t));
    // void *stream = buffer->stream;
    //  Deserializamos los campos que tenemos en el buffer
    memcpy(&entero32, buffer->stream + buffer->offset, sizeof(uint32_t));
    // stream += sizeof(uint32_t);
    buffer->offset += sizeof(uint32_t);

    return entero32;
}

uint8_t leer_buffer_enteroUint8(tipo_buffer *buffer)
{
    int entero8;

    // void *stream = buffer->stream;
    //  Deserializamos los campos que tenemos en el buffer
    memcpy(&entero8, buffer->stream + buffer->offset, sizeof(uint8_t));
    // stream += sizeof(uint8_t);
    buffer->offset += sizeof(uint8_t);

    return entero8;
}

char *leer_buffer_string(tipo_buffer *buffer)
{
    char *cadena;
    uint32_t tamanio;

    tamanio = leer_buffer_enteroUint32(buffer);
    cadena = malloc((tamanio) + 1);
    memcpy(cadena, buffer->stream + buffer->offset, (tamanio));
    buffer->offset += tamanio;

    *(cadena + tamanio) = '\0';

    return cadena;
}

t_list *leer_buffer_instrucciones(tipo_buffer *buffer)
{
    t_list *lista_instrucciones;
    uint32_t tamanio;
    tamanio = leer_buffer_enteroUint32(buffer);

    return lista_instrucciones;
}

t_registros *leer_buffer_registros(tipo_buffer *buffer)
{
    t_registros *registros = malloc(sizeof(t_registros));

    registros->AX = leer_buffer_enteroUint8(buffer);
    registros->BX = leer_buffer_enteroUint8(buffer);
    registros->CX = leer_buffer_enteroUint8(buffer);
    registros->DX = leer_buffer_enteroUint8(buffer);
    registros->EAX = leer_buffer_enteroUint32(buffer);
    registros->EBX = leer_buffer_enteroUint32(buffer);
    registros->ECX = leer_buffer_enteroUint32(buffer);
    registros->EDX = leer_buffer_enteroUint32(buffer);
    registros->SI = leer_buffer_enteroUint32(buffer);
    registros->DI = leer_buffer_enteroUint32(buffer);
    return registros;
}

t_cde *leer_cde(tipo_buffer *buffer)
{
    t_cde *cde = malloc(sizeof(t_cde));
    cde->pid = leer_buffer_enteroUint32(buffer);
    cde->PC = leer_buffer_enteroUint32(buffer);
    // scde->path = leer_buffer_string(buffer);
    cde->registros = malloc(sizeof(t_registros));
    cde->registros = leer_buffer_registros(buffer);
    return cde;
}
void agregar_cde_buffer(tipo_buffer *buffer, t_cde *cde)
{
    // t_list *lista_instrucciones;
    agregar_buffer_para_enterosUint32(buffer, cde->pid);
    agregar_buffer_para_enterosUint32(buffer, cde->PC);
    // agregar_buffer_para_string(buffer, cde->path);
    agregar_buffer_para_registros(buffer, cde->registros);
}
