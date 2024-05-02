#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
//include <utils.h>
#include <sockets.c>
#include "../../utils/include/utils.h"
#include "../include/conexiones.h"
#include  "../../utils/include/instrucciones.h"

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

struct config_cpu *config_cpu()
{
	struct config_cpu *valores_config = malloc(sizeof(struct config_cpu));

	// creo el config
	valores_config->config = iniciar_config("cpu.config");
	valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP_MEMORIA");
	valores_config->ip_kernel = config_get_string_value(valores_config->config, "IP_KERNEL");
	valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_MEMORIA");
	valores_config->puerto_escucha_dispatch = config_get_string_value(valores_config->config, "PUERTO_ESCUCHA_DISPATCH");

	return valores_config;
}

//INSTRUCCIONES

void ejecutarCicloInstruccion(int instruccion, uint32_t PC);
void tipoInstruccion(int instruccion);

#endif