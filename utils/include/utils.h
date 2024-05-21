#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
// #include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <semaphore.h>
// #include <sys/socket.h>
#include <commons/collections/list.h>
#include <assert.h>
#include "registros.h"
#include "sockets.h"
typedef enum
{
	ERROR_CLIENTE_DESCONECTADO = -1,
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
	PEDIDO_PCB,
	EJECUTAR_PROCESO, // CPU
	INTERRUPT,
	DESALOJO,
	SOLICITUD_INTERFAZ_GENERICA,
	NO_ESTOY_LIBRE,
	CONCLUI_OPERACION,
	ESTOY_LIBRE,
	CONSULTAR_DISPONIBILDAD,
	PROCESO_INTERRUMPIDO,
	SOLICITUD_EXIT, // cortoPlazo
	SOLICITUD_INTERFAZ_STDIN,
	SOLICITUD_INTERFAZ_STDOUT,
	SOLICITUD_INTERFAZ_DIALFS,
	ENVIAR_INSTRUCCION_CORRECTO
} op_code;

typedef struct
{
	t_log *logger;
	char *puerto;
	char *ip;
} t_args;
typedef enum
{
	NEW,
	READY,
	EXEC,
	BLOCKED,
	FINISHED

} t_estados;
typedef struct
{
	t_cde *cde;		  // contexto de ejecucion
	t_estados estado; // estado del proceso
	// int prioridad no va por ahora
	t_list *archivosAsignados;
	t_list *recursosAsignados;
	int prioridad;
} t_pcb;
extern sem_t *sem_kernel;
extern sem_t *sem_kernel_io_generica;
extern t_log *logger;
void iterator(char *value);
t_log *iniciar_logger(char *path_log, char *nombre_log);
t_config *iniciar_config(char *config_path);
void leer_consola(t_log *);
void iniciar_sem_globales();
void terminar_programa(int, t_log *, t_config *);
void destruirLog(t_log *logger);
void destruirConfig(t_config *config);
void liberarConexion(int conexion);
t_args *crearArgumento(char *puerto, char *ip);
#endif
