#include "../include/serializacion.h"
t_log *logger;

tipo_buffer *crear_buffer()
{
    tipo_buffer *buffer = calloc(1, sizeof(tipo_buffer));

    // Verificar si la asignación de memoria fue exitosa.
    if (buffer == NULL)
    {
        // Manejo del error, por ejemplo, retornar NULL o manejar de otra manera.
        return NULL;
    }
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
    send(socket, &(buffer->size), sizeof(uint32_t), 0);

    if (buffer->size != 0)
    {
        send(socket, buffer->stream, buffer->size, 0);
    }
    else
    {
        printf("El buffer tiene size 0");
    }
}

tipo_buffer *recibir_buffer(int socket)
{
    tipo_buffer *buffer = crear_buffer();
    recv(socket, &(buffer->size), sizeof(uint32_t), MSG_WAITALL);
    if (buffer->size != 0)
    {
        buffer->stream = malloc(buffer->size);
        recv(socket, buffer->stream, buffer->size, MSG_WAITALL);
    }
    return buffer;
}

uint32_t leer_buffer_enteroUint32(tipo_buffer *buffer)
{
    int entero32;
    memcpy(&entero32, buffer->stream + buffer->offset, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    return entero32;
}

uint8_t leer_buffer_enteroUint8(tipo_buffer *buffer)
{
    int entero8;
    memcpy(&entero8, buffer->stream + buffer->offset, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
    return entero8;
}

char *leer_buffer_string(tipo_buffer *buffer)
{
    char *cadena;
    uint32_t tamanio;

    tamanio = leer_buffer_enteroUint32(buffer);
    cadena = malloc((tamanio) + 1);
    memcpy(cadena, buffer->stream + buffer->offset, tamanio);
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
    cde->registros = malloc(sizeof(t_registros));
    cde->registros = leer_buffer_registros(buffer);
    return cde;
}
void agregar_cde_buffer(tipo_buffer *buffer, t_cde *cde)
{
    agregar_buffer_para_enterosUint32(buffer, cde->pid);
    agregar_buffer_para_enterosUint32(buffer, cde->PC);
    agregar_buffer_para_registros(buffer, cde->registros);
}

void free_t_write_memoria(t_write_memoria *escribir)
{
    free(escribir->stream);
    free(escribir);
}
void agregar_t_write_memoria_buffer(tipo_buffer *buffer, t_write_memoria *escribir)
{
    if (buffer == NULL || escribir == NULL)
    {
        // Manejar el error adecuadamente, por ejemplo, retornar o imprimir un error.
        return;
    }

    size_t nuevo_tamano = buffer->size + sizeof(int) + escribir->size + sizeof(u_int32_t) + sizeof(int);
    void *temp_stream = realloc(buffer->stream, nuevo_tamano);
    if (temp_stream == NULL)
    {
        // Manejar el error de realloc, por ejemplo, liberar recursos o imprimir un error.
        return;
    }
    buffer->stream = temp_stream;

    memcpy(buffer->stream + buffer->offset, &escribir->size, sizeof(int));
    buffer->offset += sizeof(int);
    memcpy(buffer->stream + buffer->offset, escribir->stream, escribir->size);
    buffer->offset += escribir->size;
    memcpy(buffer->stream + buffer->offset, &escribir->direccion_fisica, sizeof(u_int32_t));
    buffer->offset += sizeof(u_int32_t);
    memcpy(buffer->stream + buffer->offset, &escribir->pid, sizeof(int));
    buffer->offset += sizeof(int);

    buffer->size = nuevo_tamano;
}

t_write_memoria *leer_t_write_memoria_buffer(tipo_buffer *buffer)
{
    t_write_memoria *escribir = malloc(sizeof(t_write_memoria));
    memcpy(&escribir->size, buffer->stream + buffer->offset, sizeof(int));
    buffer->offset += sizeof(int);
    escribir->stream = malloc(escribir->size);
    memcpy(escribir->stream, buffer->stream + buffer->offset, escribir->size);
    buffer->offset += escribir->size;
    memcpy(&escribir->direccion_fisica, buffer->stream + buffer->offset, sizeof(u_int32_t));
    buffer->offset += sizeof(u_int32_t);
    memcpy(&escribir->pid, buffer->stream + buffer->offset, sizeof(int));
    buffer->offset += sizeof(int);
    return escribir;
}

t_write_memoria *crear_t_write_memoria(int size, void *stream, u_int32_t direccion_fisica, int pid)
{
    t_write_memoria *escribir = malloc(sizeof(t_write_memoria));
    escribir->size = size;
    escribir->stream = malloc(size);
    memcpy(escribir->stream, stream, size);
    escribir->direccion_fisica = direccion_fisica;
    escribir->pid = pid;
    return escribir;
}