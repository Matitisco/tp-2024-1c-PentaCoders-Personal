#ifndef ENTRADA_SALIDA_H_
#define ENTRADA_SALIDA_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/include/sockets.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern t_log *logger;
int conexion_kernel,conexion_memoria_desde_IO;
int socket_memoria;
int socket_kernel;

struct config_io{
	t_config* config;
	char *ip;
	char *puerto_kernel;
	char *puerto_memoria;
	char* tipo_interfaz;
	//int tiempo_unidad_trabajo;
	char *ip_kernel;
	char *ip_memoria;
	//path_base_dialfs;
	//int block_size;
	//int block_count;
};

struct config_io* config_io(){
	struct config_io* valores_config = malloc(sizeof(struct config_io));

	//creo el config
	valores_config->config = iniciar_config("entrada_salida.config");

	valores_config->ip = config_get_string_value(valores_config->config, "IP_KERNEL");
	valores_config->puerto_kernel = config_get_string_value(valores_config->config, "PUERTO_KERNEL");
	valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_MEMORIA");

    
	return valores_config;
}

#endif
