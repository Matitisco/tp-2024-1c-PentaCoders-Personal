#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sockets.c>
#include <pthread.h>

#include "../../utils/include/sockets.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/instrucciones.h"
#include "../../utils/include/serializacion.h"
#include "../../utils/include/registros.h"
#include "../../kernel/include/kernel.h"
#include "../include/diccionario.h"

char *PUERTO_MEMORIA;
int CONEXION_A_MEMORIA;
t_registros *registros;
t_dictionary *diccionario_instrucciones;
// t_log *logger;
int socket_memoria;
int socket_kernel_dispatch;
int socket_kernel_interrupt;

// Variable Global
uint32_t pid_ejecutar;

// Semaforos
pthread_mutex_t mutex_cde_ejecutando;

struct config_cpu
{
	t_config *config;
	char *ip_memoria;
	char *ip_kernel;
	char *puerto_memoria;
	char *puerto_escucha_dispatch;
	char *puerto_escucha_interrupt;
	// char* algoritmo_tlb;
	// int cantidad_entradas_tlb;
};

struct config_cpu *config_cpu();

// INSTRUCCIONES

void ejecutarCicloInstruccion(int instruccion, uint32_t PC);
void tipoInstruccion(int instruccion);
void servidorDeKernel(struct config_cpu *valores_config);
void proceso_dispatch(void *socket_server);
void ejecutar_proceso();
void solicitar_instruccion();
void iniciar_registros();
void pedir_instruccion_a_memoria();
#endif