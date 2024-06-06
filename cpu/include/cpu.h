#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <sockets.c>
#include <pthread.h>

// #include "../../utils/include/sockets.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/instrucciones.h"
#include "../../utils/include/serializacion.h"
#include "../../utils/include/registros.h"
#include "../../kernel/include/kernel.h"

extern int socket_kernel_dispatch;
//  int socket_kernel_interrupt;
extern t_registros *registros;
//extern config_cpu *valores_config_cpu;
extern int interrupcion_io;
extern tipo_buffer *buffer_instruccion_io;
extern int tamanio_pagina;
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
extern int salida_exit;
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
t_cde *buscar_cde(uint32_t pid);

// CICLO DE INSTRUCCION
char *fetch(t_cde *contexto);
char **decode(char *linea_de_instrucion);
void execute(char **instruccion, t_cde *contextoProceso);
t_tipoDeInstruccion obtener_instruccion(char *instruccion);
void actualizar_cde(t_cde *contexto);

// CONEXIONES Y SERVIDORES
void *conexionAMemoria(void *ptr);
void levantar_Kernel_Dispatch(void *ptr);
void levantar_Kernel_Interrupt(void *ptr);

// INSTRUCCIONES
void exec_set(char *registro, uint32_t valor);
void exec_mov_in(char *datos, char *direccion);
void exec_mov_out(char *direccion, char *datos);
void exec_sum(char *destino, char *origen);
void exec_sub(char *destino, char *origen);
void exec_jnz(char *registro, uint32_t numeroInstruccion);
void exec_resize(char * tamanio,t_cde *contextoProceso);
void exec_copy_string();
void exec_wait();
void exec_signal();
void exec_io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo);
void exec_io_stdin_read();
void exec_io_stdout_write();
void exec_io_fs_create();
void exec_io_fs_delete();
void exec_io_fs_truncate();
void exec_io_fs_write();
void exec_io_fs_read();
void exec_exit(t_cde *cde);

void check_interrupt();

#endif