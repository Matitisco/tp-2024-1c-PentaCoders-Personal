#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/instrucciones.h"
#include "../../utils/include/serializacion.h"
#include "../../utils/include/registros.h"
#include "../../kernel/include/kernel.h"

// VARIABLES
extern int socket_kernel_dispatch;
extern int tamanio_pagina;
extern int desalojo_wait;
extern int desalojo_signal;
extern int interrupcion_io;
extern int salida_exit;
extern int TLB_HABILITADA;
extern t_cde *cde_recibido;
extern t_registros *registros;
extern tipo_buffer *buffer_instruccion_io;

// ESTRUCTURAS
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

// INICIALIZACION
void iniciar_modulo_cpu();
void tlb_iniciar(char *algoritmo, int cant_entradas);
void iniciar_semaforos_CPU();
void iniciar_registros_sistema();
void recibir_tamanio_pagina(int socket_memoria);
config_cpu *configurar_cpu();

// CONEXIONES Y SERVIDORES
void levantar_conexion_a_memoria();
void *levantar_kernel_dispatch();
void *levantar_kernel_interrupt();

// HILOS
void crear_hilos_CPU();

// CICLO DE INSTRUCCION
char *fetch(t_cde *contexto);
char **decode(char *linea_de_instrucion);
t_tipoDeInstruccion obtener_instruccion(char *instruccion);
void execute(char **instruccion, t_cde *contextoProceso);
void actualizar_cde(t_cde *contexto);
void check_interrupt();

// INSTRUCCIONES
void exec_set(char *registro, uint32_t valor);
void exec_mov_in(char *datos, char *direccion, t_cde *cde);
void exec_mov_out(char *direccion, char *datos, t_cde *cde);
void exec_sum(char *destino, char *origen);
void exec_sub(char *destino, char *origen);
void exec_jnz(char *registro, uint32_t numeroInstruccion, t_cde *cde);
void exec_resize(char *tamanio, t_cde *cde);
void exec_copy_string(char *tamanio, t_cde *cde);
void exec_wait(char *recurso, t_cde *cde);
void exec_signal(char *recurso, t_cde *cde);
void exec_io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo);
void exec_io_stdin_read(char *interfaz, char *reg_direccion, char *reg_tamanio, t_cde *cde);
void exec_io_stdout_write(char *interfaz, char *reg_direccion, char *reg_tamanio, t_cde *cde);
void exec_io_fs_create();
void exec_io_fs_delete();
void exec_io_fs_truncate();
void exec_io_fs_write();
void exec_io_fs_read();
void exec_exit(t_cde *cde);

#endif