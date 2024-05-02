#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include "../include/conexiones.h"
#include "../../utils/include/utils.h"
#include <stdint.h>
#include "../../utils/include/registros.h"
#include "../../utils/include/instrucciones.h"
#include <semaphore.h>
#include <pthread.h>
#include "../../utils/include/serializacion.h"

typedef struct
{
	t_log *logger;
	char *puerto;
	char *ip;
} t_args ;

extern t_log *logger;
int socket_cpu;
pthread_t hiloCPU, hiloKernel, hiloIO;
t_args *args_CPU;
t_args *args_KERNEL;
t_args *args_IO;
struct config_memoria
{
	t_config *config;
	char *ip_memoria;
	char *puerto_memoria;
	// int tam_memoria;
	// int tam_pagina;
	// char* path_instrucciones;
	// int retardo_respuesta;
};
t_args* crearArgumento(char *puerto, char *ip);
void crearHilos(t_args *args_CPU, t_args *args_IO, t_args *args_KERNEL);
struct config_memoria *config_memoria();
void *recibirCPU(void* ptr);
void *recibirKernel(void *ptr);
void *recibirIO(void* ptr);

#endif