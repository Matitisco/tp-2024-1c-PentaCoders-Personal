#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/temporal.h>
#include <semaphore.h>
#include <pthread.h>

// #include "cortoPlazo.h"
// #include "menu.h"
// #include "operaciones.h"

#include "../../utils/include/utils.h"
#include "../../utils/include/registros.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/serializacion.h"
#include "../../utils/include/instrucciones.h"

extern int QUANTUM;

// ENUMS

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
	char *algoritmo_planificacion;
	int quantum;
	t_list *listaRecursos;
	t_list *instanciasRecursos;
	int grado_multiprogramacion;
} config_kernel;

// QUITAR ENUM_INTERFAZ UNA VEZ MERGEADO INTERFACES Y KERNEL
typedef enum
{
	GENERICA,
	STDIN,
	STDOUT,
	DIALFS,
} enum_interfaz;

typedef struct
{
	enum_interfaz tipo_interfaz;
	int tiempo_unidad_trabajo;
	char *ip_kernel;
	int puerto_kernel;
	char *ip_memoria;
	int puerto_memoria;
	char *path_base_dialfs;
	int block_size;
	int block_count;
	int retraso_compactacion;
} t_interfaz;

typedef struct
{
	char *nombreEstado;
	t_queue *estado;
	pthread_mutex_t *mutex_estado;

} colaEstado;




// VARIABLES
extern uint32_t PID_GLOBAL;
extern t_log *logger;
extern sem_t* GRADO_MULTIPROGRAMACION;
extern sem_t* procesos_en_new;
extern sem_t* procesos_en_ready;
extern sem_t* procesos_en_exec;
extern sem_t* procesos_en_block;
extern sem_t* procesos_en_exit;
extern sem_t* sem_agregar_a_estado;
extern sem_t *sem_kernel;
extern sem_t *sem_kernel_io_generica;
//Binarios
extern sem_t *binario_menu_lp;
extern sem_t *b_largo_plazo_exit;

extern sem_t *exec_libre;
extern sem_t *b_reanudar_largo_plazo;
extern int habilitar_largo_plazo;





extern config_kernel *valores_config;

extern int socket_memoria;
extern int socket_cpu_dispatch;
extern int socket_cpu_interrupt;
extern int socket_io;
// DECLARACION VARIABLES GLOBALES

extern colaEstado *cola_new_global;
extern colaEstado *cola_ready_global;
extern colaEstado *cola_exec_global;
extern colaEstado *cola_bloqueado_global;
extern colaEstado *cola_exit_global;

// FUNCIONES
void crearHilos(t_args *args_MEMORIA, t_args *args_IO, t_args *args_CPU_DS, t_args *args_CPU_INT);
void *conexionAMemoria(void *ptr);
void iniciar_consola_interactiva();
void gestionar_peticiones_memoria();
void gestionar_peticiones_interfaces();
void planificar_ejecucion_procesos();
void inicializarEstados();
colaEstado *constructorColaEstado(char *nombre);
config_kernel *inicializar_config_kernel();

void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado,  sem_t* contador_estado);
t_pcb* sacar_procesos_cola(colaEstado *cola_estado, sem_t* contador_estado);

void iniciar_semaforos();

// FUNCIONES DE LEVANTAR MODULOS
void *levantar_CPU_Dispatch(void *ptr);
void *levantar_CPU_Interrupt(void *ptr);
void *levantarIO(void *ptr);
void iniciar_hilos(config_kernel *valores_config);

// planificadores

void *largo_plazo();
void *transicion_exit_largo_plazo();


void *corto_plazo();


// funciones de io

bool se_encuentra_conectada(char *elem_lista, char *interfaz_nombre);
char *obtener_interfaz(enum_interfaz interfaz);
void recibir_orden_interfaces_de_cpu(char **interfaces);

#endif