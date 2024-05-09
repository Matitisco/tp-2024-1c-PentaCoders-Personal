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

char *PUERTO_MEMORIA;
int CONEXION_A_MEMORIA;
//t_log *logger;
int socket_memoria;
int socket_kernel_dispatch;
int socket_kernel_interrupt;

//Variable Global
uint32_t pid_ejecutar;

//Semaforos
pthread_mutex_t mutex_cde_ejecutando;

typedef struct 
{
	t_config *config;
	char *ip;
	char *puerto_memoria;
	char *puerto_escucha_dispatch;
	char *puerto_escucha_interrupt;
	char* algoritmo_tlb;
	int cantidad_entradas_tlb;
}config_cpu;

//INSTRUCCIONES

void ejecutarCicloInstruccion(int instruccion, uint32_t PC);
void tipoInstruccion(int instruccion);
void servidorDeKernel(config_cpu *valores_config_cpu);
void proceso_dispatch(int socket_servidor_dispatch);
t_cde *leer_payload_PCB(tipo_buffer *buffer);
void ejecutar_proceso();
void solicitar_instruccion();
config_cpu *configurar_cpu();

#endif