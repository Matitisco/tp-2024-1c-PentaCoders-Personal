#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <sys/socket.h>
#include <commons/collections/list.h>
#include <assert.h>
#include "../include/sockets.h"

// CLIENTE

typedef enum
{
	MENSAJE,
	PAQUETE
} op_code;

typedef struct
{
	int size;
	void *stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer *buffer;
} t_paquete;

typedef enum {
    EJECUTAR_PROCESO,
	INTERRUPT,
	DESALOJO,
	CDE,
}mensaje_kernel_cpu;

typedef enum {

	PEDIDO_INSTRUCCION

}mensaje_cpu_memoria;

typedef enum {
	SOLICITUD_INICIAR_PROCESO,
	INICIAR_PROCESO_CORRECTO,
	INICIAR_PROCESO_ERROR,
	SOLICITUD_FINALIZAR_PROCESO,
	FINALIZAR_PROCESO,
	ERROR_FINALIZAR_PROCESO

}mensaje_kernel_memoria;



void enviar_mensaje(char *mensaje, int socket_cliente);
t_paquete *crear_paquete(void);
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);
void enviar_paquete(t_paquete *paquete, int socket_cliente);

void eliminar_paquete(t_paquete *paquete);

// SERVIDOR

extern t_log *logger;

void *recibir_buffer(int *, int);

t_list *recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

// KERNEL UTILS
void iterator(char *value);

// I_O UTILS
t_log *iniciar_logger(char *path_log, char *nombre_log);
t_config *iniciar_config(char *config_path);
void leer_consola(t_log *);
void paquete(int);
void terminar_programa(int, t_log *, t_config *);

void destruirLog(t_log *logger);

void destruirConfig(t_config *config);

void liberarConexion(int conexion);

#endif
