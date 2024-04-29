
#include "../include/serializacion.h"

tipo_buffer *crear_buffer()
{
    tipo_buffer *unBuffer = malloc(sizeof(unBuffer)); // Reservo memoria
    if (unBuffer == NULL) // si de casualidad no pudo
    {
        return NULL;
    }
    else
    {
        unBuffer ->size = 0; //inicializo los valores para que no haya cosas rawras
        unBuffer->offset = 0;
        unBuffer->stream = NULL;
        return unBuffer; // lo retorno 
    }
}
void destruir_buffer(tipo_buffer *unBuffer)
{
    if(unBuffer != NULL){ //esto es mas que nada para chequear
       free(unBuffer ->stream); //primero liberamos al puntero
       free(unBuffer);//lugeo loberamos al buffer
    }
   
    // Tal vez deberia hacer free miembro a miembro
}

void escribir_buffer(tipo_buffer *buffer, uint8_t entero)
{
   buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
    if (buffer->stream == NULL) {
        // Por si hay algun error
        return;
    }
    memcpy((char *)buffer->stream + buffer->size, &entero, sizeof(uint8_t));
    buffer->size += sizeof(uint8_t);

}
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

tipo_buffer* recibir_buffer(int socket) {
    tipo_buffer* buffer = crear_buffer();
    if (buffer == NULL) {
        // Manejar error de creación de buffer
        return NULL;
    }
    // Recibo el tamaño del buffer y reservo espacio en memoria
    recv(socket, &(buffer->size), sizeof(uint32_t), MSG_WAITALL);
    if (buffer->size > 0) {
        buffer->stream = malloc(buffer->size);
        if (buffer->stream == NULL) {
            // Manejar error de malloc
            destruir_buffer(buffer);
            return NULL;
        }
        // Recibo stream del buffer
        recv(socket, buffer->stream, buffer->size, MSG_WAITALL);
    }
    return buffer;
}
