#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <libgen.h>

#include "../../utils/include/serializacion.h"
#include "../include/conexiones.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/registros.h"
#include "../../utils/include/instrucciones.h"


typedef struct
{
	int pid;
	t_list *paginas_proceso;
} t_tabla_paginas;


typedef struct
{
	int marco;		 // numero de marco donde esta la pagina
} t_pagina;

typedef struct
{
	int numero_marco; // numero de marco
	int bit_ocupado;  // esta libre o no el marco
} t_bit_map;

typedef struct{
    int socket_io;
    char* nombre_io;
}t_info_io_memoria;


extern t_log *logger;

int socket_cpu;
int cliente_fd;
pthread_t hiloCPU, hiloKernel, hiloIO;

t_list *list_tabla_paginas;

int cant_marcos;
t_pagina *crear_pagina(int marco);
t_tabla_paginas *buscar_en_lista_global(int pid);
int obtener_posicion_marco_libre();
uint32_t hay_marco_libre();
t_list *agregar_pagina(t_pagina *pagina, t_list *list_paginas);
void crear_y_agregar_tabla_a_lista_global(int pid);

void crearHilos();
config_memoria *configuracion_memoria();
void *recibirCPU();
void *recibirKernel();
void hay_marcos_suficientes(int paginas_adicionales, int cliente_cpu);
int cantidad_marcos_libres();
void imprimir_tabla_de_paginas_proceso(t_tabla_paginas *tabla_paginas_proceso);
void *recibir_interfaz_io();
void inicializar_bitmap(int cant_marcos);
void iniciar_proceso(int cliente_fd);
void finalizar_proceso(int cliente_fd);
char *obtener_ruta(char *nombre_archivo);
void eliminar_proceso(int pid_a_eliminar);
t_pcb *buscar_proceso_por_pid(int pid);
void pedido_instruccion_cpu_dispatch(int cliente_fd);
t_list *leerArchivoConInstrucciones(char *nombre_archivo);
t_cde *obtener_contexto_en_ejecucion(int PID);
t_instruccion *crearInstruccion(char *linea);
_Bool estaElContextoConCiertoPID(t_cde *contexto);
char *obtener_char_instruccion(t_tipoDeInstruccion instruccion_code);
t_cde *armarCde(tipo_buffer *buffer);

void obtener_y_eliminar_cde(int pid, t_registros *reg);
bool coincidePID(void *pid);
void element_destroyer(void *element);
void destroyer_instruccion(void *element);
void eliminar_cde(int pid);
void liberar_registros(t_registros *registros);

void *destroy_instruccion(void *element);

// espacio de usuario
void *leer_memoria(uint32_t numero_pagina, uint32_t offset, uint32_t pid, uint32_t tamanio);
void *escribir_memoria(uint32_t numero_pagina, uint32_t offset, uint32_t pid, void *valor_a_escribir, uint32_t tamanio);

char *leer_memoria_stdout(int32_t direccion_fisica, uint32_t pid, int limite_bytes);
void *acceso_a_espacio_usuario();
void pedido_frame_mmu(int cliente_cpu);
void escritura_interfaz(tipo_buffer *buffer, int cliente_solicitante);
void lectura_interfaz(tipo_buffer *buffer_lectura, int cliente_solicitante);
void escritura_cpu(tipo_buffer *buffer, int cliente_solicitante);
void lectura_cpu(tipo_buffer *buffer_lectura, int cliente_solicitante);
// resize
void reducir_proceso(uint32_t pid, uint32_t tamanio, int cliente_cpu);
void ampliar_proceso(uint32_t pid, uint32_t tamanio, int cliente_cpu);
void asignar_paginas_nuevas(t_tabla_paginas *tabla_paginas, int paginas_adicionales, uint32_t pid);
void eliminar_paginas(t_list *paginas, int cantidad_a_eliminar);
// auxx de paginas
int consultar_marco_de_una_pagina(t_tabla_paginas *tabla, int nroPagina);
void enviar_tamanio_pagina(int cpu);
void liberar_marco(int nroMarco);
int obtener_posicion_marco_libre();

void colocar_pagina_en_marco(t_pagina *pagina);
t_tabla_paginas *buscar_en_lista_global(int pid);
void eliminar_tabla_paginas(uint32_t pid);
void destruir_pagina(void *pagina);
t_list *agregar_pagina(t_pagina *pagina, t_list *list_paginas);
int tamanio_proceso(int pid);

void liberar_marco(int nroMarco);
void imprimir_estado_marcos();
void *conexiones_io();
#endif