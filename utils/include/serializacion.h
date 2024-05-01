#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "instrucciones.h"
#include "registros.h"
#include <commons/collections/list.h>
typedef struct
{
    uint32_t size;
    uint32_t offset;
    void *stream;

} tipo_buffer;

// FUNCIONES DEL BUFFER
tipo_buffer *crear_buffer();
void destruir_buffer(tipo_buffer *unBuffer);
void escribir_buffer(tipo_buffer *buffer, uint8_t entero);
void enviar_buffer(tipo_buffer *buffer, int socket);

uint32_t leer_buffer_entero(tipo_buffer *buffer);
t_instruccion *leer_buffer_instruccion(tipo_buffer *buffer);
void paramBufferALista(int cantParam, tipo_buffer *buffer);
t_registros *leer_buffer_registros(tipo_buffer *buffer);
char *leer_buffer_string(tipo_buffer *buffer, uint32_t *tam);
void *leer_buffer_pagina(tipo_buffer *buffer, uint32_t tamPagina);
// FUNCIONES DE COD OP
void enviar_cod_enum(int socket_servidor, uint32_t cod);
uint32_t recibir_cod(int socket_cliente);

tipo_buffer *recibir_buffer_propio(int socket);

#endif