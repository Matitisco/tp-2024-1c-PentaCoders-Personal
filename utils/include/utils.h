#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <sys/socket.h>
#include <commons/collections/list.h>
#include <assert.h>
#include "sockets.h"
typedef struct
{
    uint32_t size;
    uint32_t offset;
    void *stream;

} tipo_buffer;

typedef enum
{
	MENSAJE, 
	PAQUETE,
	SOLICITUD_INICIAR_PROCESO,
	INICIAR_PROCESO_CORRECTO,
	INICIAR_PROCESO_ERROR,
	SOLICITUD_FINALIZAR_PROCESO,
	FINALIZAR_PROCESO,
	EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    INICIAR_PLANIFICACION,
    DETENER_PLANIFICACION,
    LISTAR_ESTADOS,
	ERROR_FINALIZAR_PROCESO,
	PEDIDO_INSTRUCCION,
	PEDIDO_PCB
} op_code;


typedef struct
{
	op_code codigo_operacion;
	tipo_buffer *buffer;
} t_paquete;

typedef enum {
    EJECUTAR_PROCESO,
	INTERRUPT,
	DESALOJO,
	CDE,
}mensaje_kernel_cpu;



void enviar_mensaje(char *mensaje, int socket_cliente);
t_paquete *crear_paquete(void);
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);
void enviar_paquete(t_paquete *paquete, int socket_cliente);

void eliminar_paquete(t_paquete *paquete);

// SERVIDOR

extern t_log *logger;


//void *recibir_buffer(int *, int);

t_list *recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

// KERNEL UTILS
void iterator(char *value);

// I_O UTILS
t_log *iniciar_logger(char *path_log, char *nombre_log);
t_config *iniciar_config(char *config_path);
void leer_consola(t_log *);
void paquete(int);
void terminar_programa(int, t_log *, t_config *);

void destruirLog(t_log *logger);

void destruirConfig(t_config *config);

void liberarConexion(int conexion);


t_paquete *crear_paquete(void);

#endif
