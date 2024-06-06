#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/collections/list.h>

#include "instrucciones.h"
#include "registros.h"
#include "utils.h"

typedef struct
{
    uint32_t size;
    uint32_t offset;
    void *stream;

} tipo_buffer;

typedef struct
{
    op_code codigo_operacion;
    tipo_buffer *buffer;
} t_paquete;

void enviar_mensaje(char *mensaje, int socket_cliente);
void recibir_mensaje(int socket_cliente);
t_paquete *crear_paquete(void);
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
void eliminar_paquete(t_paquete *paquete);
t_list *recibir_paquete(int socket_cliente);
void *serializar_paquete(t_paquete *paquete, int bytes);
void paquete(int conexion);


void enviar_cod_enum(int socket_servidor, uint32_t cod);
op_code recibir_operacion(int socket_cliente);

tipo_buffer *crear_buffer();

void agregar_buffer_para_enterosUint32(tipo_buffer *buffer, uint32_t entero);
void agregar_buffer_para_enterosUint8(tipo_buffer *buffer, uint8_t entero);
void agregar_buffer_para_string(tipo_buffer *buffer, char *args);
void agregar_buffer_para_registros(tipo_buffer *buffer, t_registros *registros);

void enviar_buffer(tipo_buffer *buffer, int socket);
tipo_buffer *recibir_buffer(int socket);

uint32_t leer_buffer_enteroUint32(tipo_buffer *buffer);
uint8_t leer_buffer_enteroUint8(tipo_buffer *buffer);
char *leer_buffer_string(tipo_buffer *buffer);
t_registros* leer_buffer_registros(tipo_buffer *buffer);

void destruir_buffer(tipo_buffer *buffer);
/* ---------CDE ------*/
t_cde *leer_cde(tipo_buffer *buffer);

t_list *recibir_paquete_cde(int socket_cliente);

t_paquete *crear_paquete_cde(void);

void agregar_escrituraMemoria_buffer(tipo_buffer * buffer,t_escrituraMemoria valores);
t_escrituraMemoria leer_escrituraMemoria(tipo_buffer *buffer);
/* op_code recibir_operacion2(int socket_cliente); */

#endif