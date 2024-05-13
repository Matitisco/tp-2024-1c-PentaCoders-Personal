#include "../include/kernel.h"

colaEstado *cola_new_global;
colaEstado *cola_ready_global;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;
int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;
int socket_interfaz;

// Contadores Semaforos
sem_t *GRADO_MULTIPROGRAMACION;
sem_t *procesos_en_new;
sem_t *procesos_en_ready;
sem_t *procesos_en_exec;
sem_t *procesos_en_block;
sem_t *procesos_en_exit;
sem_t *exec_libre;

// Semaforos de binarios
sem_t *binario_menu_lp;
sem_t *sem_agregar_a_estado;

int QUANTUM;
pthread_t hiloMEMORIA;
pthread_t hiloIO;
pthread_t hiloLargoPlazo;
pthread_t hiloCortoPlazo;
pthread_t hiloCPUINT;
pthread_t hiloCPUDS;
pthread_t hiloConsola;

t_args *args_MEMORIA;
t_args *args_IO;
t_args *args_KERNEL;
t_args *args_CPU_DS;
t_args *args_CPU_INT;
extern t_log *logger;

config_kernel *valores_config;

int main(int argc, char *argv[])
{
	inicializarEstados();

	logger = iniciar_logger("kernel.log", "KERNEL");

	valores_config = inicializar_config_kernel();

	iniciar_semaforos();

	iniciar_hilos(valores_config);

	pthread_join(hiloMEMORIA, NULL);
	pthread_join(hiloLargoPlazo, NULL);
	pthread_join(hiloCortoPlazo, NULL);
	/* 	pthread_join(hiloCPUDS, NULL);
		pthread_join(hiloCPUINT, NULL);*/
	// pthread_join(hiloIO, NULL);
	pthread_join(hiloConsola, NULL);
	//  LIBERAR COSAS
	liberar_conexion(socket_memoria);
}

void iniciar_hilos(config_kernel *valores_config)
{
	args_MEMORIA = crearArgumento(valores_config->puerto_memoria, valores_config->ip_memoria);
	args_IO = crearArgumento(valores_config->puerto_escucha, valores_config->ip_memoria);
	args_CPU_DS = crearArgumento(valores_config->puerto_cpu_dispatch, valores_config->ip_cpu);
	args_CPU_INT = crearArgumento(valores_config->puerto_cpu_interrupt, valores_config->ip_cpu);
	crearHilos(args_MEMORIA, args_IO, args_CPU_DS, args_CPU_INT);
}
void crearHilos(t_args *args_MEMORIA, t_args *args_IO, t_args *args_CPU_DS, t_args *args_CPU_INT)
{
	pthread_create(&hiloMEMORIA, NULL, conexionAMemoria, (void *)args_MEMORIA);
	// pthread_create(&hiloIO, NULL, levantarIO, (void *)args_IO);
	/* 	pthread_create(&hiloCPUDS, NULL, levantar_CPU_Dispatch, (void *)args_CPU_DS);
		pthread_create(&hiloCPUINT, NULL, levantar_CPU_Interrupt, (void *)args_CPU_INT); */
	pthread_create(&hiloLargoPlazo, NULL, largo_plazo, NULL);
	pthread_create(&hiloCortoPlazo, NULL, corto_plazo, NULL);
	pthread_create(&hiloConsola, NULL, iniciar_consola_interactiva, NULL);
}
/* void *levantarIO(void *ptr)
{
	t_args *argumento = malloc(sizeof(t_args));
	argumento = (t_args *)ptr;
	tipo_buffer *buffer_io;
	int server_fd = iniciar_servidor(logger, "Kernel", argumento->ip, argumento->puerto);
	log_info(logger, "Servidor KERNEL listo para recibir Interfaces de IO");
	t_list *lista_interfaces = list_create();
	while (1)
	{
		// Debemos aplicar semaforos?? muy probable
		int cliente_fd = esperar_cliente(logger, "Kernel", server_fd);
		buffer_io = recibir_buffer(cliente_fd);
		// ACORDARSE DE LUEGO BORRAR LA ESTRCUTURA QUE SE NECUENTRA EN KERNEL.H TENEMOS QUE SOLO USAR LA QUE ESTA EN ENTRADA Y SALIDA.H
		char *nombre_IO = leer_buffer_string(buffer_io);
		enum_interfaz cod_io = leer_buffer_enteroUint32(buffer_io);
		char *tipo_io = obtener_interfaz(cod_io);
		if (list_find(lista_interfaces, se_encuentra_conectada))
		{
			list_add(lista_interfaces, nombre_IO);
		}
		else
		{
			log_info(logger, "Esta Interfaz ya se encuentra conectada");
		}
		log_info(logger, "Me llego una interfaz del tipo:%s, y de nombre:%s", tipo_io, nombre_IO);
	}
	return NULL;
} */
const char *obtener_interfaz(enum_interfaz interfaz)
{
	if (interfaz == GENERICA)
	{
		return "GENERICA";
	}
	else if (interfaz == STDIN)
	{
		return "STDIN";
	}
	else if (interfaz == STDOUT)
	{
		return "STDOUT";
	}
	else if (interfaz == DIALFS)
	{
		return "DIALFS";
	}
	return "NO SE ENTIENDE LA INTERFAZ ENVIADA";
}
bool se_encuentra_conectada(char *elem_lista, char *interfaz_nombre)
{
	return strcmp(elem_lista, interfaz_nombre) == 0;
}

void *conexionAMemoria(void *ptr)
{
	t_args *argumento = malloc(sizeof(t_args));
	argumento = (t_args *)ptr;
	socket_memoria = levantarCliente(logger, "MEMORIA", argumento->ip, argumento->puerto, "KERNEL SE CONECTO A MEMORIA");
	free(argumento);
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

void inicializarEstados()
{
	cola_new_global = constructorColaEstado("NEW");
	cola_ready_global = constructorColaEstado("READY");
	cola_exec_global = constructorColaEstado("EXEC");
	cola_bloqueado_global = constructorColaEstado("BLOCK");
	cola_exit_global = constructorColaEstado("EXIT");
}

void iniciar_semaforos()
{
	GRADO_MULTIPROGRAMACION = malloc(sizeof(sem_t));
	procesos_en_new = malloc(sizeof(sem_t));
	procesos_en_ready = malloc(sizeof(sem_t));
	procesos_en_exec = malloc(sizeof(sem_t));
	procesos_en_block = malloc(sizeof(sem_t));
	procesos_en_exit = malloc(sizeof(sem_t));
	binario_menu_lp = malloc(sizeof(sem_t));
	sem_agregar_a_estado = malloc(sizeof(sem_t));

	sem_init(GRADO_MULTIPROGRAMACION, 0, valores_config->grado_multiprogramacion);
	sem_init(procesos_en_new, 0, 0);
	sem_init(procesos_en_ready, 0, 0);
	sem_init(procesos_en_exec, 0, 0);
	sem_init(procesos_en_block, 0, 0);
	sem_init(procesos_en_exit, 0, 0);
	sem_init(binario_menu_lp, 0, 0);
	sem_init(sem_agregar_a_estado, 0, 0);
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
	configuracion->algoritmo_planificacion = config_get_string_value(configuracion->config, "ALGORITMO_PLANIFICACION");
	configuracion->quantum = config_get_int_value(configuracion->config, "QUANTUM");
	configuracion->grado_multiprogramacion = config_get_int_value(configuracion->config, "GRADO_MULTIPROGRAMACION");
	// configuracion->listaRecursos = list_create();
	// configuracion->listaRecursos = string_get_string_as_array((config_get_string_value(configuracion->listaRecursos, "RECURSOS")));
	// configuracion->instanciasRecursos = list_create();
	// RECURSOS=[RA,RB,RC]
	// INSTANCIAS_RECURSOS=[1,2,1]
	return configuracion;
}

void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado, sem_t *contador_estado) // Añade un proceso a la cola New //MONITOR DE QUEUE_PUSH
{
	pthread_mutex_lock(cola_estado->mutex_estado);
	queue_push(cola_estado->estado, pcb);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	sem_post(contador_estado);
}

t_pcb *sacar_procesos_cola(colaEstado *cola_estado, sem_t *contador_estado)
{
	t_pcb *pcb = malloc(sizeof(pcb));
	sem_wait(contador_estado);
	pthread_mutex_lock(cola_estado->mutex_estado);
	pcb = queue_pop(cola_estado->estado);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	return pcb;
}

void *levantar_CPU_Interrupt(void *ptr)
{
	return NULL;
}
void *levantar_CPU_Dispatch(void *ptr)
{
	t_args *datosConexion = malloc(sizeof(t_args));
	datosConexion = (t_args *)ptr;
	int socket_cpu = levantarCliente(logger, "CPU", datosConexion->ip, datosConexion->puerto, "KERNEL SE CONECTO A CPU");

	free(datosConexion);
}
void recibirPedidoDeIOGENSLEEP(){
	
op_code mensaje =recibir_operacion( socket_cpu_dispatch);
	if(mensaje == SOLICITUD_INTERFAZ_GENERICA){
	tipo_buffer *buffer = recibir_buffer(socket_cpu_dispatch);
	  int socket_kernel; // declaracion momentania
    enviar_cod_enum(socket_kernel, SOLICITUD_INTERFAZ_GENERICA);
     char * nombre_interfaz =leer_buffer_string(buffer);
	 uint32_t unidades_trabajo = leer_buffer_enteroUint32(buffer);
	 tipo_buffer*bufferInterfaz = crear_buffer();
	enviar_cod_enum(socket_interfaz, SOLICITUD_INTERFAZ_GENERICA);
    agregar_buffer_para_string(buffer, nombre_interfaz);
    agregar_buffer_para_enterosUint32(buffer, unidades_trabajo);
    enviar_buffer(buffer, socket_kernel); // falta definir quien es cliente_kernel

}

}
/*


 IO_GEN_SLEEP (Interfaz, Unidades de trabajo): Esta instrucción solicita al Kernel que se envíe
a una interfaz de I/O a que realice un sleep por una cantidad de unidades de trabajo.



*/
