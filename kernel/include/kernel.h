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
#include <libgen.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/registros.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/serializacion.h"
#include "../../utils/include/instrucciones.h"

typedef t_list t_lista_recursos;

typedef struct
{
	char *nombreEstado;
	t_list *estado;
	pthread_mutex_t *mutex_estado;
	sem_t *contador;

} colaEstado;

typedef struct
{
	char *nombre;
	sem_t *instancias;
	colaEstado *cola_bloqueados;
} t_recurso;

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
	t_lista_recursos *recursos;
	int grado_multiprogramacion;

} config_kernel;

typedef struct
{
	int cliente_io;
	char *nombre_io;
	enum_interfaz tipo_IO;
	t_list *procesos_espera;
} t_infoIO;

// VARIABLES
extern uint32_t PID_GLOBAL;
extern t_log *logger;
extern sem_t *GRADO_MULTIPROGRAMACION;
extern sem_t *sem_agregar_a_estado;
extern sem_t *sem_kernel_io_generica;

extern int QUANTUM;
extern pthread_t hiloQuantum;
extern t_cde *cde_interrumpido;
// Binarios
extern sem_t *sem_quantum;
extern sem_t *binario_menu_lp;
extern sem_t *b_largo_plazo_exit;
extern sem_t *b_exec_libre;
extern sem_t *b_transicion_exec_blocked;
extern sem_t *b_reanudar_largo_plazo;
extern sem_t *b_reanudar_corto_plazo;
extern sem_t *b_transicion_exec_ready;
extern sem_t *b_transicion_blocked_ready;
extern sem_t *b_detener_planificacion;

extern int habilitar_planificadores;
extern config_kernel *valores_config;
extern int socket_memoria;
extern int socket_cpu_dispatch;
extern sem_t *sem_finalizar_proceso;
extern int socket_cpu_interrupt;
// DECLARACION VARIABLES GLOBALES

// INTERRUPTORES
extern sem_t *b_switch_readys;
extern int interruptor_switch_readys;

extern sem_t *contador_readys;

extern colaEstado *cola_new_global;
extern colaEstado *cola_ready_global;
extern colaEstado *cola_ready_plus;
extern colaEstado *cola_exec_global;
extern colaEstado *cola_bloqueado_global;
extern colaEstado *cola_exit_global;

extern sem_t *bloquearReady;
extern sem_t *bloquearReadyPlus;

// FUNCIONES
void crear_hilos();
void proceso_estado();
void conexionAMemoria();
void iniciar_consola_interactiva();
void iniciar_consola_interactiva2();
void gestionar_peticiones_memoria();
void gestionar_peticiones_interfaces();
void planificar_ejecucion_procesos();
void inicializarEstados();
colaEstado *constructorColaEstado(char *nombre);
config_kernel *inicializar_config_kernel();

void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado);
t_pcb *sacar_procesos_cola(colaEstado *cola_estado);


t_pcb *transicion_generica(colaEstado *colaEstadoInicio, colaEstado *colaEstadoFinal, char *planificacion);
void evaluar_planificacion(char *planificador);

void iniciar_semaforos();

// FUNCIONES DE LEVANTAR MODULOS
void levantar_CPU_Dispatch();
void levantar_CPU_Interrupt();
void *levantarIO();
void iniciar_hilos();

// planificadores

void *largo_plazo();
void *transicion_exit_largo_plazo();
void *transicion_exec_ready();
void *corto_plazo();
void *hilo_quantum();
void *transicion_exec_blocked();
void *transicion_blocked_ready();
void iniciar_kernel();
void levantar_servidores();
int existe_recurso(int *posicion);
bool existe_recurso2(char *nombre_recurso);

void wait_instancia_recurso(int i);

void wait_instancia_recurso2(t_recurso *recurso);
_Bool ya_tiene_instancias_del_recurso(void *ptr);

void signal_instancia_recurso(t_recurso *recurso);
void interfaz_no_conectada(int pid);
void eliminar_proceso(t_pcb *proceso);
t_pcb *buscarProceso(uint32_t pid);
void finalizar_proceso(uint32_t PID, motivoFinalizar motivo);

// FUNCIONES DE ENTRADA/SALIDA

char *obtener_interfaz(enum_interfaz interfaz);
_Bool interfaz_esta_en_lista(void *ptr);
void recibir_orden_interfaces_de_cpu(int pid, tipo_buffer *buffer_con_instruccion);
_Bool interfaz_no_esta_conectada(t_infoIO *informacion_interfaz);
t_cde *iniciar_cde(char *PATH);
void interfaz_conectada_stdin(t_tipoDeInstruccion instruccion_a_ejecutar, int tamanio_reg, int dir_fisica, int socket_io, int pid);
void interfaz_conectada_stdout(t_tipoDeInstruccion instruccion_a_ejecutar, int tamanio_reg, int dir_fisica, int socket_io, int pid);
void interfaz_conectada_generica(int unidades_trabajo, t_tipoDeInstruccion instruccion_a_ejecutar, t_infoIO *io, int pid);
void atender_interrupciones();
char *buscar_recurso(char *recurso, int *posicion);

int hayProcesosEnEstado(colaEstado *cola_estado);

void bloquearSiReadysVacios();
void desbloquearSiReadysVacios();

void waitInterruptor(int valor_interruptor, sem_t *interruptorSemaforo);

void signalInterruptor(int valor_interruptor, sem_t *interruptorSemaforo); // no se usa

void valorSemaforo(sem_t *semaforo);

bool existe_recurso2(char *nombre_recurso);

t_recurso *obtener_recurso(char *nombre_recurso);

#endif