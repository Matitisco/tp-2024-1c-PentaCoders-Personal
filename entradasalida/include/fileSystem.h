#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <commons/txt.h>
#include "entrada_salida.h"


//y el archivo de bloques normal
//bit map que nos dice los bloques libres
//metaData - utilizar las config 

char*path_meta_data;
char*path_bitmap;
char*path_arch_bloques;
int bloque_inicial ;
int tamanio_meta_data;
t_bitarray* bitarray;
FILE*arch_bloque;
FILE*bit_map;
typedef struct t_bitarray
{
    /* data */
};

void instrucciones_dialfs();
//operaciones con archivos
void escribir_archivo(char *nombre_archivo, tipo_buffer *buffer, uint32_t pid);
void leer_archivo(char *nombre_archivo, tipo_buffer *buffer, uint32_t pid);
void eliminar_archivo(char *nombre_archivo, uint32_t pid);
void truncar_archivo(char *nombre_archivo, tipo_buffer *buffer, uint32_t pid);
void crear_archivo(char *nombre_archivo, uint32_t pid);


int liberarBloque(uint32_t bit);
t_config *buscar_meta_data(char *nombre_archivo);
int contar_bloques_ocupados_bitarray();
void sacar_bloque(uint32_t bloques_a_eliminar, uint32_t tamanio_a_aplicar);
void agregar_bloque(uint32_t bloques_agregar, uint32_t tamanio_a_aplicar);
_Bool espacio_disponible_es_contiguo(uint32_t cantidad_bloques_agregar, uint32_t tamanio_bloque, uint32_t bloque_inicial, uint32_t tamanio_archivo_anterior);
_Bool hay_espacio_disponible(uint32_t cant_bloques_agregar);
void reducir_archivo(t_config* archivo_metadata_buscado, uint32_t tamanio_archivo_anterior, uint32_t tamanio_a_aplicar, uint32_t tamanio_bloque);
void ampliar_archivo(t_config* archivo_meta_data_buscado,uint32_t tamanio_a_aplicar ,uint32_t tamanio_archivo_anterior,uint32_t cantidad_bloques_agregar,uint32_t tamanio_bloque, uint32_t bloque_inicial);
void cambiar_tamanio_archivo(char *nombre_archivo, uint32_t nuevo_tamanio);


uint32_t bloque_libre();

#endif




