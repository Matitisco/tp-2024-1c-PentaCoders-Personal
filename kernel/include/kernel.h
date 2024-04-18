#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/src/utils.h"
#include "../include/conexiones.h"


int conexion_memoria,conexion_cpu;

t_log *logger;
struct config_kernel {
	t_config* config;
    char* puerto_escucha;
	char* ip_memoria;
	char* puerto_memoria;
	char* ip_cpu;
	char*puerto_cpu_dispatch;
	char *puerto_cpu_interrupt;	
};

struct config_kernel* config_kernel(){
	struct config_kernel* configuracion = (struct config_kernel*) malloc(sizeof(struct config_kernel));

	configuracion->config = iniciar_config("kernel.config");

	configuracion->ip_memoria = config_get_string_value(configuracion->config,"IP_MEMORIA");
	configuracion->ip_cpu = config_get_string_value(configuracion->config,"IP_CPU");
	configuracion->puerto_memoria = config_get_string_value(configuracion->config,"PUERTO_MEMORIA");
	configuracion->puerto_escucha = config_get_string_value(configuracion->config,"PUERTO_ESCUCHA");
	configuracion->puerto_cpu_dispatch = config_get_string_value(configuracion->config,"PUERTO_CPU_DISPATCH");
	configuracion->puerto_cpu_interrupt= config_get_string_value(configuracion->config,"PUERTO_CPU_INTERRUPT");

	return configuracion;
}


#endif