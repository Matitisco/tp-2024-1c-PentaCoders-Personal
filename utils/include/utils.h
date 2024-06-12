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
	SOLICITUD_INICIAR_PROCESO,
	INICIAR_PROCESO_CORRECTO,
	ERROR_INICIAR_PROCESO,
	SOLICITUD_FINALIZAR_PROCESO,
	FINALIZAR_PROCESO,
	// MENU
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
	DIRECCION_CORRECTA
} op_code;
typedef enum
{
	GENERICA,
	STDIN,
	STDOUT,
	DIALFS,
} enum_interfaz;

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
	t_list *archivosAsignados;
	t_list *recursosAsignados;
	int prioridad;
  / t_list *lista_paginas; // cada proceso tiene su lista de paginas-la tabla de paginas
	int quantum;
} t_pcb;


typedef struct
{
	uint32_t valor;
	uint32_t direccion_fisica;
} t_escrituraMemoria;
int string_to_int(char *str);


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