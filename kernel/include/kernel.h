#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/src/utils.h"
#include "../include/registros.h"
#include "../include/conexiones.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <pthread.h>
// t_log *logger;

// ENUMS

typedef enum
{
	NEW,
	READY,
	EXEC,
	BLOCKED,
	FINISHED

} t_estados;

typedef enum
{
	FIFO,
	RR,
	VRR
} t_alg_planificadores;

// ESTRUCTURAS

// CONFIG KERNEL
struct config_kernel
{
	t_config *config;
	char *puerto_escucha;
	char *ip_memoria;
	char *puerto_memoria;
	char *ip_cpu;
	char *puerto_cpu_dispatch;
	char *puerto_cpu_interrupt;
};

// INICIALIZAR CONFIG KERNEL
struct config_kernel *inicializar_config_kernel();

// PCB
typedef struct 
{
	t_cde *cde;		  // contexto de ejecucion
	t_estados estado; // estado del proceso
	// int prioridad no va por ahora
	//int pid;	 // lo dejamos en registros.h
	int quantum; // unidad de tiempo usada en VRR
}t_pcb;

// COLAS

typedef struct
{
	t_queue *estado;
	pthread_mutex_t *mutex_estado;
} colaEstado;

// DECLARACION VARIABLES GLOBALES

extern colaEstado *cola_new_global;
extern colaEstado *cola_ready_global;
extern colaEstado *cola_exec_global;
extern colaEstado *cola_bloqueado_global;
extern colaEstado *cola_exit_global;

// CONSTRUCTOR/INICIALIZADOR COLAS ESTADOS

colaEstado *constructorColaEstado();

// INICIALIZAR 5 COLAS ESTADOS
void inicializarEstados();

// FUNCIONES

void iniciar_consola_interactiva(t_log *logger);
void gestionar_peticiones_memoria();
void gestionar_peticiones_interfaces();
void planificar_ejecucion_procesos();
void mostrar_operaciones_realizables();

#endif