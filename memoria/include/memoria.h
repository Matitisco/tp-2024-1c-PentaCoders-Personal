#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>

#include "../../utils/include/serializacion.h"
#include "../include/conexiones.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/registros.h"
#include "../../utils/include/instrucciones.h"

extern t_log *logger;
int socket_cpu;
pthread_t hiloCPU, hiloKernel, hiloIO;
extern sem_t *sem_kernel;
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
void crearHilos();
struct config_memoria *config_memoria();
void *recibirCPU();
void *recibirKernel();
void *recibirIO();
void iniciar_sem_globales();
void iniciar_proceso(int cliente_fd, tipo_buffer *buffer);
void finalizar_proceso(int cliente_fd, tipo_buffer *buffer);
void eliminar_proceso(int pid_a_eliminar);
t_pcb *buscar_proceso_por_pid(int pid);
t_list *leerArchivoConInstrucciones(char *nombre_archivo);
t_instruccion *crearInstruccion(char *linea);
t_cde* armarCde(tipo_buffer* buffer);

#endif