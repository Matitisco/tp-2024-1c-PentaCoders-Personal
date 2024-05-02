#include "../include/kernel.h"

colaEstado *cola_new_global;
colaEstado *cola_ready_global;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;
int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;
int  QUANTUM;
extern t_log *logger;

//t_log *logger;
//uint32_t PID_GLOBAL;

int main(int argc, char *argv[])
{
	int conexion_memoria_desde_kernel, conexion_cpu;

	// INICIALIZACION DE ESTADOS
	inicializarEstados();

	logger = iniciar_logger("kernel.log", "KERNEL");

	// CONFIG
	config_kernel *valores_config = inicializar_config_kernel();

	// KERNEL COMO SERVER DE I0
	levantarServidor(logger, valores_config->puerto_escucha, valores_config->ip_memoria, "SERVIDOR KERNEL");
	// KERNEL COMO CLIENTE
	conexion_memoria_desde_kernel = levantarCliente(logger, "MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "KERNEL SE CONECTO A MEMORIA");
	conexion_cpu = levantarCliente(logger, "CPU", valores_config->ip_cpu, valores_config->puerto_cpu_dispatch, "KERNEL SE CONECTO A CPU");

	iniciar_consola_interactiva(logger);
	terminar_programa(conexion_cpu, logger, valores_config->config);
	liberarConexion(conexion_memoria_desde_kernel);
}

void gestionar_peticiones_memoria()
{
	printf("Gestionar Peticion Memoria");
}

void gestionar_peticiones_interfaces()
{
	printf("Gestionar Peticion Interfaces");
}

void planificar_ejecucion_procesos()
{
	printf("Planificar Ejecucion procesos");
}

colaEstado *constructorColaEstado(char *nombre)
{
	colaEstado *cola_estado_generica;
	cola_estado_generica = malloc(sizeof(colaEstado));
	cola_estado_generica->nombreEstado = nombre;
	// strcpy(cola_estado_generica->nombreEstado, nombre);
	cola_estado_generica->estado = queue_create();
	cola_estado_generica->mutex_estado = malloc(sizeof(pthread_mutex_t));

	pthread_mutex_init(cola_estado_generica->mutex_estado, NULL);

	return cola_estado_generica;
}

// INICIALIZAR COLAS DE ESTADOS
void inicializarEstados()
{
	cola_new_global = constructorColaEstado("NEW");
	cola_ready_global = constructorColaEstado("READY");
	cola_exec_global = constructorColaEstado("EXEC");
	cola_bloqueado_global = constructorColaEstado("BLOCK");
	cola_exit_global = constructorColaEstado("EXIT");
}

config_kernel *inicializar_config_kernel()
{
	config_kernel *configuracion = (config_kernel *)malloc(sizeof(config_kernel));

	configuracion->config = iniciar_config("kernel.config");

	configuracion->ip_memoria = config_get_string_value(configuracion->config, "IP_MEMORIA");
	configuracion->ip_cpu = config_get_string_value(configuracion->config, "IP_CPU");
	configuracion->puerto_memoria = config_get_string_value(configuracion->config, "PUERTO_MEMORIA");
	configuracion->puerto_escucha = config_get_string_value(configuracion->config, "PUERTO_ESCUCHA");
	configuracion->puerto_cpu_dispatch = config_get_string_value(configuracion->config, "PUERTO_CPU_DISPATCH");
	configuracion->puerto_cpu_interrupt = config_get_string_value(configuracion->config, "PUERTO_CPU_INTERRUPT");

	return configuracion;
}
