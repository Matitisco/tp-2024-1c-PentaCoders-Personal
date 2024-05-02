#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/registros.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/serializacion.h"
#include "../../utils/include/instrucciones.h"

// VARIABLES
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

typedef struct
{
	t_config *config;
	char *puerto_escucha;
	char *ip_memoria;
	char *puerto_memoria;
	char *ip_cpu;
	char *puerto_cpu_dispatch;
	char *puerto_cpu_interrupt;
} config_kernel;

typedef struct
{
	t_cde *cde;		  // contexto de ejecucion
	t_estados estado; // estado del proceso
	// int prioridad no va por ahora
	t_list *archivosAsignados;
	t_list *recursosAsignados;
	char *path;
	int prioridad;
} t_pcb;
typedef struct
{
	char *nombreEstado;
	t_queue *estado;
	pthread_mutex_t *mutex_estado;

} colaEstado;

extern int socket_memoria;
extern int socket_cpu_dispatch;
extern int socket_cpu_interrupt;

// DECLARACION VARIABLES GLOBALES

extern colaEstado *cola_new_global;
extern colaEstado *cola_ready_global;
extern colaEstado *cola_exec_global;
extern colaEstado *cola_bloqueado_global;
extern colaEstado *cola_exit_global;

// FUNCIONES

void iniciar_consola_interactiva(t_log *logger);
void gestionar_peticiones_memoria();
void gestionar_peticiones_interfaces();
void planificar_ejecucion_procesos();
void inicializarEstados();
colaEstado *constructorColaEstado(char *nombre);
config_kernel *inicializar_config_kernel();

// CORTO PLAZO


#endif