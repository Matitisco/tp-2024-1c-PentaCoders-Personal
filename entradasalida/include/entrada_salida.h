#ifndef ENTRADA_SALIDA_H_
#define ENTRADA_SALIDA_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/include/sockets.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../../utils/include/serializacion.h"

extern t_log *logger;
int conexion_kernel, conexion_memoria_desde_IO;
int socket_memoria;
int socket_kernel;
int estoy_libre;

/*

	PRUEBA CON DIALFS QUE ANDA TODO BIEN Y SE LEEN LOS ARCHIVOS
	printf("%d\n", interfaz_io->tipo_interfaz);
	printf("%d\n", interfaz_io->tiempo_unidad_trabajo);
	// printf("%d\n", interfaz_io->retraso_compactacion);
	// printf("%d\n", interfaz_io->puerto_memoria);
	printf("%d\n", interfaz_io->puerto_kernel);
	// printf("%s\n", interfaz_io->path_base_dialfs);
	// printf("%s\n", interfaz_io->ip_memoria);
	printf("%s\n", interfaz_io->ip_kernel);
	// printf("%d\n", interfaz_io->block_size);
	// printf("%d\n", interfaz_io->block_count);

*/
typedef struct
{
	t_config *config;
	// char *ip;
	int puerto_kernel;
	int puerto_memoria;
	char *tipo_interfaz;
	char *ip_kernel;
	char *ip_memoria;
	char *path_base_dialfs;
	int tiempo_unidad_trabajo;
	int block_size;
	int block_count;
	int retraso_compactacion;
} config_io;


config_io *inicializar_config_IO(char *PATH);
void levantar_interfaz(char *nombre, char *PATH);
t_interfaz *crear_interfaz_generica(config_io *config, char *nombre);
t_interfaz *crear_interfaz_stdin(config_io *config, char *nombre);
t_interfaz *crear_interfaz_stdout(config_io *config, char *nombre);
t_interfaz *crear_interfaz_dialfs(config_io *config, char *nombre);
enum_interfaz asignar_interfaz(char *nombre_Interfaz);
void arrancar_interfaz_generica(t_interfaz *interfaz_io);
void arrancar_interfaz_stdin(t_interfaz *interfaz_io);
void arrancar_interfaz_stdout(t_interfaz *interfaz_io);
void arrancar_interfaz_dialfs(t_interfaz *interfaz_io);
void realizar_operacion_gen();
void realizar_operacion_stdin();
int pasar_a_int(enum_interfaz tipo_int);
#endif
