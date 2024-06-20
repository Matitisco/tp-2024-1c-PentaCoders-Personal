#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
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

#endif




