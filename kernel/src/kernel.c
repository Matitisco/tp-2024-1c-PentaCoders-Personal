#include "../include/kernel.h"

colaEstado *cola_new_global;
colaEstado *cola_ready_global;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;
int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;

int QUANTUM;
pthread_t hiloMEMORIA;
t_args *args_MEMORIA;
extern t_log *logger;

// pthread_t hiloCPU;
// pthread_t hiloMEMORIA;

int main(int argc, char *argv[])
{
	// INICIALIZACION DE ESTADOS
	inicializarEstados();

	logger = iniciar_logger("kernel.log", "KERNEL");

	// CONFIG
	config_kernel *valores_config = inicializar_config_kernel();

	// KERNEL COMO SERVER DE I0
	// levantarServidor(logger, valores_config->puerto_escucha, valores_config->ip_memoria, "SERVIDOR KERNEL");
	// KERNEL COMO CLIENTE
	// SOCKET MEMORIA SE DEBE QUEDAR ASI SINO NO SE ENVIA NADA DESDE KERNEL A MEMORIA
	args_MEMORIA = crearArgumento(valores_config->puerto_memoria, valores_config->ip_memoria);
	crearHilos(args_MEMORIA);
	// socket_cpu_dispatch = levantarCliente(logger, "CPU", valores_config->ip_cpu, valores_config->puerto_cpu_dispatch, "KERNEL SE CONECTO A CPU");
	// pthread_create(hiloCPU);
	pthread_join(hiloMEMORIA, NULL);
	iniciar_consola_interactiva(logger);

	// terminar_programa(socket_cpu_dispatch, logger, valores_config->config);

	liberarConexion(socket_memoria);
}
void crearHilos(t_args *args_MEMORIA)
{
	pthread_create(&hiloMEMORIA, NULL, enviarAMemoria, (void *)args_MEMORIA);
}

void *enviarAMemoria(void *ptr)
{
	t_args *argumento = malloc(sizeof(t_args));
	argumento = (t_args *)ptr;
	socket_memoria = levantarCliente(logger, "MEMORIA", argumento->ip, argumento->puerto, "KERNEL SE CONECTO A MEMORIA");
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
