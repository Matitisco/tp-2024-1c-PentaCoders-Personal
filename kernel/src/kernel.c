#include "../include/kernel.h"

colaEstado *cola_new_global;
colaEstado *cola_ready_global;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;
int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;

// Contadores Semaforos
sem_t *GRADO_MULTIPROGRAMACION;
sem_t *procesos_en_new;
sem_t *procesos_en_ready;
sem_t *procesos_en_exec;
sem_t *procesos_en_block;
sem_t *procesos_en_exit;

// Semaforos de binarios
sem_t *binario_menu_lp;
sem_t *sem_agregar_a_estado;

int QUANTUM;
pthread_t hiloMEMORIA;
pthread_t hiloIO;
pthread_t hiloLargoPlazo;
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
	// iniciar_consola_interactiva(logger);

	pthread_join(hiloMEMORIA, NULL);
	pthread_join(hiloLargoPlazo, NULL);
	// pthread_join(hiloCPUDS, NULL);
	// pthread_join(hiloCPUINT, NULL);
	pthread_join(hiloIO, NULL);
	// pthread_join(hiloConsola, NULL);
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
	pthread_create(&hiloLargoPlazo, NULL, largo_plazo, NULL);
	// pthread_create(&hiloIO, NULL, levantarIO, (void *)args_IO);
	// pthread_create(&hiloCPUDS, NULL, levantar_CPU_Dispatch, (void *)args_CPU_DS);
	// pthread_create(&hiloCPUINT, NULL, levantar_CPU_Interrupt, (void *)args_CPU_INT);
	pthread_create(&hiloConsola, NULL, iniciar_consola_interactiva, NULL);
}
void *levantarIO(void *ptr)
{
	/* 	t_args *argumento = malloc(sizeof(t_args));
		argumento = (t_args *)ptr;
		int server_fd = iniciar_servidor(logger, "Kernel", argumento->ip, argumento->puerto);
		log_info(logger, "Servidor KERNEL listo para recibir Interfaces de IO");
		t_list *lista_interfaces = list_create();
		while (1)
		{
			// Debemos aplicar semaforos?? muy probable
			// vamos a necesitar un hilo exclusivo para esperar al cliente?? es, decir que controle constantemente quien viene?
			// vamos a necesitar semaforos si fuera asi, ya que debemos controlar quien llega y quien se va
			int cliente_fd = esperar_cliente(logger, "Kernel", server_fd);
			// SE SUPONE QUE LA INTERFAZ ENVIA UN MENSAJE CON SUS ATRIBUTOS (PAQUETE)
			// SE RECIBE UN PAQUETE CON LOS VALORES DE LA INTERFAZ
			t_list *valores_interfaz_conectada = recibir_paquete(cliente_fd); // extraigo el paquete y meto todo en una lista;
			// ACORDARSE DE LUEGO BORRAR LA ESTRCUTURA QUE SE NECUENTRA EN KERNEL.H TENEMOS QUE SOLO USAR LA QUE ESTA EN ENTRADA Y SALIDA.H
			char *nombre_IO = obtenerNombreInterfaz(valores_interfaz_conectada);
			if (list_find(lista_interfaces, se_encuentra_conectada, nombre_IO))
			{
				list_add(lista_interfaces, nombreIO);
			}
			else
			{
				log_info(logger, "Esta Interfaz ya se encuentra conectada");
			}
			// log_info(logger, "Me llego una interfaz del tipo:%s, y de nombre:%s",parametros);
			list_clean_and_destroy_elements(valores_interfaz_conectada);
		} */
	return NULL;
}
char *obtenerNombreInterfaz(t_list *lista)
{
	return "A";
}
bool se_encuentra_conectada(void *lista, void *interfaz_nombre)
{
	t_list *lista_recorrer = (t_list *)lista;
	char *elemento = (char *)interfaz_nombre;

	return lista != NULL && strcmp(lista_recorrer->head->data, elemento) == 0;
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
	binario_menu_lp= malloc(sizeof(sem_t));
	sem_agregar_a_estado =  malloc(sizeof(sem_t));;

	sem_init(GRADO_MULTIPROGRAMACION, 0, valores_config->grado_multiprogramacion);
	sem_init(procesos_en_new, 0, 0);
	sem_init(procesos_en_ready, 0, 0);
	sem_init(procesos_en_exec, 0, 0);
	sem_init(procesos_en_block, 0, 0);
	sem_init(procesos_en_exit, 0, 0);
	sem_init(binario_menu_lp,0,0);	
	sem_init(sem_agregar_a_estado, 0,0);
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
	//RECURSOS=[RA,RB,RC]
	//INSTANCIAS_RECURSOS=[1,2,1]
	return configuracion;
}

// pcb 0x7fffe8001300  y cde 0x7fffe80012d0

void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado, sem_t *contador_estado) // Añade un proceso a la cola New //MONITOR DE QUEUE_PUSH
{
	//sem_wait();
	//sem_wait(contador_estado);
	pthread_mutex_lock(cola_estado->mutex_estado);
	queue_push(cola_estado->estado, pcb); // region critica
	pthread_mutex_unlock(cola_estado->mutex_estado);
	sem_post(contador_estado);//contador_estado =1
}

// void funcion_referencia(int &x);
// void intercambiar(int &a, int &b);

t_pcb* sacar_procesos_cola(colaEstado *cola_estado, sem_t *contador_estado)	//MONITOR DE QUEUE_POP
{
	t_pcb* pcb= malloc(sizeof(pcb));
	sem_wait(contador_estado); //
	//sem_wait (sem_agregar_a_estado); // 
	pthread_mutex_lock(cola_estado->mutex_estado);
	pcb = queue_pop(cola_estado->estado);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	//sem_post(sem_agregar_a_estado);
	//sem_post(contador_estado);
	//sem_post();
	return pcb;
}
/*void *levantar_CPU_Interrupt(void *ptr)
{
	return NULL;
}
void *levantar_CPU_Dispatch(void *ptr)
{
	t_args *datosConexion = malloc(sizeof(t_args));
	datosConexion = (t_args *)ptr;
	int socket_cpu = levantarCliente(logger, "CPU", datosConexion->ip, datosConexion->puerto, "KERNEL SE CONECTO A CPU");

	free(datosConexion);
}*/
