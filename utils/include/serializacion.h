#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct {
uint32_t size;
uint32_t offset;
void *stream;

}tipo_buffer;

//FUNCIONES DLE BUFFER
tipo_buffer*crear_buffer();
void destruir_buffer(tipo_buffer *unBuffer);
void escribir_buffer(tipo_buffer *buffer, uint8_t entero);
void enviar_buffer(tipo_buffer *buffer, int socket);
tipo_buffer* recibir_buffer(int socket);





#endif