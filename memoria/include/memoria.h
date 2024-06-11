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
int cliente_fd;
pthread_t hiloCPU, hiloKernel, hiloIO;
typedef struct
{
	t_config *config;
	char *ip_memoria;
	char *puerto_memoria;
	int tam_memoria;
	int tam_pagina;
	char *path_instrucciones;
	int retardo_respuesta;
} config_memoria;

typedef struct
{
	int pid;
	t_list *tabla_paginas_proceso;
} t_tabla_paginas;

void *espacio_memoria;
typedef struct
{
	int marco;			// numeor de marco donde esta la pagina
	int bit_presencia;	// si esta en memoroa
	int bit_modificado; // si la pag fue modifica
	int pid;			// la pagina conoce al proceso del cual es parte ?
} t_pagina;

/*Paginacion */
t_list *list_tabla_paginas; // losta global de paginas y cada nodo tiene una tabla del proceso

int cant_marcos;
t_pagina *crear_pagina(int bit_presencia, int marco, int pidProceso);
void eliminar_tabla_paginas(uint32_t pid);
t_tabla_paginas *buscar_en_lista_global(int pid);
int obtener_marco_libre();
uint32_t hay_marco_libre();
t_list *agregar_pagina(t_pagina *pagina, t_list *list_paginas);
void crear_y_agregar_tabla_a_lista_global(int pid);
int *agarro_marco_que_este_libre();

typedef struct
{
	int numero_marco; // numero de marco
	int bit_ocupado;	  // esta libre o no el marco
} t_bit_map;

t_args *crearArgumento(char *puerto, char *ip);
void crearHilos();
config_memoria *configuracion_memoria();
void *recibirCPU();
void *recibirKernel();
void *recibir_interfaces_io();
void inicializar_bitmap(int cant_marcos);
// void iniciar_sem_globales();
void iniciar_proceso(int cliente_fd, tipo_buffer *buffer);
void finalizar_proceso(int cliente_fd, tipo_buffer *buffer);
char *obtener_ruta(char *nombre_archivo);
void eliminar_proceso(int pid_a_eliminar);
t_pcb *buscar_proceso_por_pid(int pid);
void pedido_instruccion_cpu_dispatch(int cliente_fd, t_list *contextos);
t_list *leerArchivoConInstrucciones(char *nombre_archivo);
t_cde *obtener_contexto_en_ejecucion(int PID, t_list *contextos);
t_instruccion *crearInstruccion(char *linea);
_Bool estaElContextoConCiertoPID(t_cde *contexto);
char *obtener_char_instruccion(t_tipoDeInstruccion instruccion_code);
t_cde *armarCde(tipo_buffer *buffer);

void obtener_y_eliminar_cde(int pid);
void eliminar_cde(t_cde *cde);
void liberar_registros(t_registros *registros);
void finalizar_proceso(int kernel, tipo_buffer *buffer);

void *destroy_instruccion(void *element);
char *guardar_texto_en(int direccion_fisica, char *texto);
char *leer_texto_en(int direccion_fisica, int limite);

#endif