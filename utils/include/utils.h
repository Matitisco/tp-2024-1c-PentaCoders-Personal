#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <semaphore.h>
#include <commons/collections/list.h>
#include <assert.h>
#include "registros.h"
#include "sockets.h"
typedef t_list t_lista_recursos;

typedef enum
{
	SUCCESS,
	INVALID_RESOURCE,
	INVALID_INTERFACE,
	OUT_OF_MEMORY_END,
	INTERRUPTED_BY_USER
} motivoFinalizar;

typedef enum
{
	EJECUTAR_SCRIPT,
	INICIAR_PROCESO,
	FINALIZAR_PROCESO_MENU,
	INICIAR_PLANIFICACION,
	DETENER_PLANIFICACION,
	MULTIPROGRAMACION,
	LISTAR_ESTADOS,
} opciones_menu;

typedef enum
{
	SOLICITUD_INICIAR_PROCESO,
	INICIAR_PROCESO_CORRECTO,
	ERROR_INICIAR_PROCESO,
	SOLICITUD_FINALIZAR_PROCESO,
	FINALIZAR_PROCESO,
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
	ENVIAR_INSTRUCCION_CORRECTO,
	FIN_DE_QUANTUM, // cpu, kernel
	BLOQUEADO_POR_IO,
	INTERRUPCION,
	SOLICITUD_CONEXION_IO,
	EJECUCION_IO_GEN_SLEEP_EXITOSA,
	PROCESO_INTERRUMPIDO_QUANTUM,
	INSTRUCCION_INTERFAZ,
	ESTA_CONECTADO,
	ESTABA_CONECTADO,
	AMPLIACION_PROCESO,
	REDUCION_PROCESO,
	ACCESO_ESPACIO_USUARIO,
	NO_ESTABA_CONECTADO,
	PEDIDO_ESCRITURA,
	PEDIDO_ESCRITURA_CORRECTO,
	ERROR_PEDIDO_ESCRITURA,
	PEDIDO_LECTURA,
	PEDIDO_LECTURA_CORRECTO,
	ERROR_PEDIDO_LECTURA,
	OUT_OF_MEMORY,
	RESIZE_EXTEND,
	ESCRITURA_CPU,
	OK,
	SIGNAL_RECURSO,
	WAIT_RECURSO,
	DIRECCION_CORRECTA,
	RESIZE_EXITOSO,
	SOLICITUD_ESCRITURA_CPU,
	LECTURA_CPU,
	SOLICITUD_DIALFS,
	CREAR_ARCHIVO_OK,
	ERROR_CREAR_ARCHIVO_OK,
	LECTURA_DIALFS,
	PEDIDO_FRAME,
	PEDIDO_FRAME_CORRECTO,
	PEDIDO_FRAME_INCORRECTO,
	PEDIDO_ESCRITURA_DIALFS,
	SOLICITUD_ESCRITURA_DIALFS,
	CONFIRMAR_CONEXION
} op_code;

typedef enum
{
	GENERICA,
	STDIN,
	STDOUT,
	DIALFS,
} enum_interfaz;

typedef enum {
	F_CREATE,
	F_DELETE,
	F_TRUNCATE,
	F_READ,
	F_WRITE
} operacion_dialfs;
typedef struct
{
	enum_interfaz tipo_interfaz;
	int tiempo_unidad_trabajo;
	char *ip_kernel;
	int puerto_kernel;
	char *ip_memoria;
	int puerto_memoria;
	char *path_base_dialfs;
	int block_size;
	int block_count;
	int retraso_compactacion;
	char *nombre_interfaz;
} t_interfaz;

typedef enum
{
	NEW,
	READY,
	READY_PLUS,
	EXEC,
	BLOCKED,
	FINISHED
} t_estados;
typedef struct
{
	t_cde *cde;		  // contexto de ejecucion
	t_estados estado; // estado del proceso
	t_list *archivosAsignados;
	t_lista_recursos *recursosAsignados;
	int quantum;
} t_pcb;

int string_to_int(char *str);

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
void enviar_op_code(int socket_servidor, uint32_t cod);
op_code recibir_op_code(int socket_cliente);
void sleep_ms(int milliseconds);

#endif