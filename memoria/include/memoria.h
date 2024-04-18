#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <utils.h>
#include "../include/conexiones.h"
#include "../../utils/src/utils.h"
 

t_log* logger;

struct config_memoria {
	t_config* config;
    char* ip_memoria;
	char* puerto_memoria;
	//int tam_memoria;
	//int tam_pagina;
	//char* path_instrucciones;
	//int retardo_respuesta;
};

struct config_memoria* config_memoria(){
	struct config_memoria* valores_config = malloc(sizeof(struct config_memoria));

	//creo el config
	valores_config->config = iniciar_config("../memoria.config");

	valores_config->ip_memoria = config_get_string_value(valores_config->config,"IP");
	valores_config->puerto_memoria = config_get_string_value(valores_config->config,"PUERTO_ESCUCHA");

	return valores_config;
}

#endif