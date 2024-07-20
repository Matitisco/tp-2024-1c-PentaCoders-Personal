#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <commons/txt.h>
#include <commons/collections/queue.h>
#include "entrada_salida.h"

// VARIABLES
void *bloquesMapeado;
char *path_meta_data;
char *path_bitmap;
char *path_arch_bloques;
int bloque_inicial;
int tamanio_meta_data;
t_bitarray *bitarray;
FILE *arch_bloque;
FILE *bit_map;

// ESTRUCTURAS
typedef struct
{
    char *nombre_archivo;
    int bloque_inicial;
    int tamanio; // estructura auxiliar de lista de arhcivos metadata
} t_archivo_data;

// FUNCIONES

// INSTRUCCIONES DIALFS
void crear_archivo(char *nombre_archivo, uint32_t pid);
void truncar_archivo(char *nombre_archivo, int nuevo_tamanio, uint32_t pid);
void eliminar_archivo(char *nombre_archivo, uint32_t pid);
void escribir_archivo(char *nombre_archivo, uint32_t tamanio, uint32_t direccion_fisica, uint32_t puntero_archivo, uint32_t pid);
void leer_archivo(char *nombre_archivo, uint32_t tamanio, uint32_t direccion_fisica, uint32_t puntero_archivo, uint32_t pid);
void cambiar_tamanio_archivo(char *nombre_archivo, uint32_t nuevo_tamanio);
void ampliar_archivo(char *nombre_archivo, t_config *archivo_meta_data_buscado, char *tamanio_a_aplicar, uint32_t tamanio_archivo_anterior, uint32_t cantidad_bloques_agregar, uint32_t tamanio_bloque, uint32_t bloque_inicial);
void reducir_archivo(t_config *archivo_metadata_buscado, uint32_t tamanio_archivo_anterior, uint32_t tamanio_a_aplicar, uint32_t tamanio_bloque);

void levantar_archivo_bloques();
t_config *crear_meta_data_archivo(char *nombre_archivo);
void instrucciones_dialfs();

char *obtener_ruta_archivo(char *nombre_archivo);
_Bool hay_espacio_disponible(uint32_t cant_bloques_agregar);
void sacar_bloque(t_config *archivo_meta_data_buscado, uint32_t bloques_a_eliminar, uint32_t tamanio_a_aplicar);
uint32_t bloque_libre();
t_config *buscar_meta_data(char *nombre_archivo);
// libera bloque del bitarray
int liberarBloque(uint32_t bit);
_Bool espacio_disponible_es_contiguo(uint32_t cantidad_bloques_agregar, uint32_t tamanio_bloque, uint32_t bloque_inicial, uint32_t tamanio_archivo_anterior);
int *obtener_bloques_archivo(char *nombre_archivo);
void compactar(char *nombre_archivo, int cantidad_bloques_agregar, t_config *metadata);
void mover_archivo_al_final_del_fs(char *nombre_archivo);
void desplazar_archivos_y_eliminar_bloques_libres();
void mover_archivo_al_primer_bloque_libre(int primer_bloque_libre, char *nombre_archivo, int cantidad_bloques_agregar, t_config *metadata);
void mover_bloque(int origen, int destino);
void actualizar_metadata_archivo(int nuevo_bloque_inicial);
_Bool buscar_por_bloque(t_archivo_data *data);
void levantar_bitmap();

#endif
