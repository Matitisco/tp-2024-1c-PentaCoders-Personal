#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <sockets.c>
#include <pthread.h>

//#include "../../utils/include/sockets.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/instrucciones.h"
#include "../../utils/include/serializacion.h"
#include "../../utils/include/registros.h"
#include "../../kernel/include/kernel.h"

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

// char *PUERTO_MEMORIA;
//  t_log *logger;
//  int socket_kernel_dispatch;
//  int socket_kernel_interrupt;
extern t_registros *registros;
// Variable Global
// extern uint32_t pid_ejecutar;

// Semaforos
// pthread_mutex_t *mutex_cde_ejecutando;

typedef struct
{
	t_config *config;
	char *ip;
	char *puerto_memoria;
	char *puerto_escucha_dispatch;
	char *puerto_escucha_interrupt;
	char *algoritmo_tlb;
	int cantidad_entradas_tlb;
} config_cpu;

// INSTRUCCIONES

void ejecutarCicloInstruccion(int instruccion, uint32_t PC);
void tipoInstruccion(int instruccion);
void servidorDeKernel(config_cpu *valores_config_cpu);
void proceso_dispatch(int socket_servidor_dispatch);
t_cde *leer_payload_PCB(tipo_buffer *buffer);
void ejecutar_proceso(t_cde *cde);
void solicitar_instruccion();
void iniciar_registros();
config_cpu *configurar_cpu();

void iniciar_semaforos_CPU();
// HILOS

void crearHilos_CPU(t_args *args_memoria, t_args *kernel_int, t_args *kernel_dis);
void iniciar_hilos_CPU(config_cpu *valores_config_cpu);

// CICLO DE INSTRUCCION
char *fetch(t_cde *contexto);
char **decode(char *linea_de_instrucion);
void execute(char **instruccion, t_cde *contextoProceso);
t_tipoDeInstruccion obtener_instruccion(char *instruccion);
void actualizar_cde(t_cde *contexto, char **instruccion);

// CONEXIONES Y SERVIDORES
void *conexionAMemoria(void *ptr);
void levantar_Kernel_Dispatch(void *ptr);
void levantar_Kernel_Interrupt(void *ptr);

void exec_set(char *registro, uint32_t valor);
void exec_mov_in();
void exec_mov_out();
void exec_sum(char *destino, char *origen);
void exec_sub(char *destino, char *origen);
void exec_jnz(char *registro, uint32_t numeroInstruccion);
void exec_resize();
void exec_copy_string();
void exec_wait();
void exec_signal();
void exec_io_gen_sleep();
void exec_io_stdin_read();
void exec_io_stdout_write();
void exec_io_fs_create();
void exec_io_fs_delete();
void exec_io_fs_truncate();
void exec_io_fs_write();
void exec_io_fs_read();
void exec_exit();

#endif