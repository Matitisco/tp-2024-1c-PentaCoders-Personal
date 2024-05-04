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
#include "sockets.h"
typedef enum
{
	MENSAJE, 
	PAQUETE,
	SOLICITUD_INICIAR_PROCESO,
	INICIAR_PROCESO_CORRECTO,
	ERROR_INICIAR_PROCESO,
	SOLICITUD_FINALIZAR_PROCESO,
	FINALIZAR_PROCESO,
	EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    INICIAR_PLANIFICACION,
    DETENER_PLANIFICACION,
	PLANIFICACION_PAUSADA,
	PLANIFICACION_EN_FUNCIONAMIENTO,
	MULTIPROGRAMACION,
    LISTAR_ESTADOS,
	ERROR_FINALIZAR_PROCESO,
	PEDIDO_INSTRUCCION,
	PEDIDO_PCB
} op_code;

typedef struct
{
	t_log *logger;
	char *puerto;
	char *ip;
} t_args ;

typedef enum {
    EJECUTAR_PROCESO,
	INTERRUPT,
	DESALOJO,
	CDE,
}mensaje_kernel_cpu;

extern t_log *logger;

void iterator(char *value);
t_log *iniciar_logger(char *path_log, char *nombre_log);
t_config *iniciar_config(char *config_path);
void leer_consola(t_log *);
void terminar_programa(int, t_log *, t_config *);
void destruirLog(t_log *logger);
void destruirConfig(t_config *config);
void liberarConexion(int conexion);
t_args *crearArgumento(char *puerto, char *ip);
#endif
