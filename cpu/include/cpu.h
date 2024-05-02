#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/include/sockets.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/instrucciones.h"
#include "../../utils/include/serializacion.h"


char *PUERTO_MEMORIA;
int CONEXION_A_MEMORIA;
//t_log *logger;
int socket_memoria;
int socket_kernel_dispatch;
int socket_kernel_interrupt;

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


//INSTRUCCIONES

void ejecutarCicloInstruccion(int instruccion, uint32_t PC);
void tipoInstruccion(int instruccion);

#endif