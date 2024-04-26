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
void envio_buffer(tipo_buffer *unBuffer,int unSocket);
void destroy_buffer(tipo_buffer*unBuffer);
tipo_buffer*recibo_buffer(int unSocket);




#endif