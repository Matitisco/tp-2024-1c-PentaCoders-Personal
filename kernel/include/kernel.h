#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <pthread.h>

//#include "cortoPlazo.h"
//#include "menu.h"
//#include "operaciones.h"



#include "../../utils/include/utils.h"
#include "../../utils/include/registros.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/serializacion.h"
#include "../../utils/include/instrucciones.h"

// VARIABLES
extern uint32_t PID_GLOBAL;

// ENUMSs
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
// MENU
void iniciar_consola_interactiva(t_log *logger);
void mostrar_operaciones_realizables();
op_code obtenerCodigo(char *opcion, t_log *logger);
void ejecutar_operacion(char *codigo_operacion, t_log *logger);
// OPERACIONES
/*Funciones del menu*/

void ejecutar_script();
void iniciar_proceso(char *PATH);
void finalizar_proceso(uint32_t PID);
void iniciar_planificacion();
void detener_planificacion();
void listar_procesos_x_estado();

/*Funciones de busuqeda del proceso*/

uint32_t mostrarPID(t_pcb *proceso);
//t_pcb *buscarProceso(uint32_t pid);
t_pcb *crear_proceso();
//t_pcb *buscarPCBEnColaPorPid(int pid_buscado, t_queue *cola, char *nombreCola);
t_cde *iniciar_cde();

// char *mostrarMotivo(enum motivoFinalizar motivo);

/*Funciones para liberar*/

void liberar_proceso(t_pcb *proceso);
void liberar_cde(t_pcb *proceso);
void liberar_recursos(t_pcb *proceso);
void liberar_archivos(t_pcb *proceso);
// CORTO PLAZO
bool proceso_completado();
t_pcb *obtener_siguiente_ready();
t_paquete *crear_paquete_op_code(int codOP);
t_cde *obtener_cde(t_pcb *proceso);
void enviar_cde(int conexion, t_cde *cde, int codOP);
void simular_ejecucion_proceso(t_pcb *proceso);
void ready_a_execute();
void agregar_lista_instrucciones_a_paquete(t_paquete *paquete, t_list *instrucciones);
void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion *instruccion);
void agregar_tipo_instruccion_a_paquete(t_paquete *paquete, t_tipoDeInstruccion tipo);
void agregar_entero_a_paquete(t_paquete *paquete, uint32_t x);
void agregar_string_a_paquete(t_paquete *paquete, char *palabra);
void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado);
void agregar_cde_a_paquete(t_paquete *paquete, t_cde *cde);

#endif