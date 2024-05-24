#include "../include/kernel.h"
extern sem_t *sem_kernel_io_generica;
colaEstado *cola_new_global;
colaEstado *cola_ready_global;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;
int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;
int socket_interfaz;
int socket_io;

// Contadores Semaforos
sem_t *GRADO_MULTIPROGRAMACION;

// Semaforos de binarios
sem_t *binario_menu_lp;
sem_t *b_largo_plazo_exit;

sem_t *b_exec_libre;

sem_t *sem_agregar_a_estado;

sem_t *b_reanudar_largo_plazo;
sem_t *b_reanudar_corto_plazo;

int habilitar_planificadores;

int QUANTUM;
pthread_t hiloMEMORIA;
pthread_t hiloIO;
pthread_t hiloLargoPlazo;
pthread_t hiloCortoPlazo;
pthread_t hiloCPUINT;
pthread_t hiloCPUDS;
pthread_t hiloConsola;
pthread_t largo_plazo_exit;

t_list *lista_interfaces;
t_args *args_MEMORIA;
t_args *args_IO;
t_args *args_KERNEL;
t_args *args_CPU_DS;
t_args *args_CPU_INT;
extern t_log *logger;
char *nombre_IO;
int servidor_para_io;
config_kernel *valores_config;

int main(int argc, char *argv[])
{
	inicializarEstados();

	logger = iniciar_logger("kernel.log", "KERNEL");
	lista_interfaces = list_create();
	valores_config = inicializar_config_kernel();
	QUANTUM = valores_config->quantum;
	iniciar_semaforos();

	iniciar_hilos(valores_config);

	pthread_join(hiloMEMORIA, NULL);
	pthread_join(hiloLargoPlazo, NULL);
	pthread_join(hiloCortoPlazo, NULL);
	pthread_join(hiloCPUDS, NULL);
	pthread_join(hiloCPUINT, NULL);
	pthread_join(hiloIO, NULL);
	pthread_join(hiloConsola, NULL);
	pthread_join(largo_plazo_exit, NULL);
	// LIBERAR COSAS
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
	servidor_para_io = iniciar_servidor(logger, "Kernel", valores_config->ip_memoria, valores_config->puerto_escucha);
	log_info(logger, "Servidor KERNEL listo para recibir Interfaces de IO");
	pthread_create(&hiloIO, NULL, levantarIO, NULL);
	pthread_create(&hiloCPUDS, NULL, levantar_CPU_Dispatch, (void *)args_CPU_DS);
	pthread_create(&hiloCPUINT, NULL, levantar_CPU_Interrupt, (void *)args_CPU_INT);
	pthread_create(&hiloLargoPlazo, NULL, largo_plazo, NULL);
	pthread_create(&hiloCortoPlazo, NULL, corto_plazo, NULL);
	pthread_create(&hiloConsola, NULL, iniciar_consola_interactiva, NULL);
	pthread_create(&largo_plazo_exit, NULL, transicion_exit_largo_plazo, NULL);
}

void *levantarIO()
{
	int disp_io = esperar_cliente(logger, "Kernel", "Interfaz IO", servidor_para_io); // se conecta una io al kernel
	while (1)
	{
		op_code operacion = recibir_operacion(disp_io);
		if (operacion == SOLICITUD_CONEXION_IO)
		{
			tipo_buffer *buffer_io = recibir_buffer(disp_io);

			int cod_io = leer_buffer_enteroUint32(buffer_io);
			nombre_IO = leer_buffer_string(buffer_io);
			char *tipo_io = obtener_interfaz(cod_io);

			if (list_find(lista_interfaces, interfaz_esta_conectada) != NULL)
			{
				log_info(logger, "Esta Interfaz ya se encuentra conectada");
			}
			else
			{
				log_info(logger, "Esta Interfaz no estaba conectada al kernel");
				list_add(lista_interfaces, nombre_IO);
			}

			log_info(logger, "Se conecto una interfaz del tipo: %s, y de nombre: %s", tipo_io, nombre_IO);

			// recibimos de cpu una instruccion
			recibir_orden_interfaces_de_cpu();
		}
		else
		{
			log_info(logger, "No entiendo la operacion enviada");
		}
	}
	return NULL;
}
_Bool interfaz_esta_conectada(char *interfaz)
{
	return strcmp(interfaz, nombre_IO) == 0;
}

char *obtener_interfaz(enum_interfaz interfaz)
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
	cola_estado_generica->contador = malloc(sizeof(sem_t));
	sem_init(cola_estado_generica->contador, 0, 0);
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

	binario_menu_lp = malloc(sizeof(sem_t));

	b_reanudar_largo_plazo = malloc(sizeof(sem_t));
	sem_agregar_a_estado = malloc(sizeof(sem_t));

	b_largo_plazo_exit = malloc(sizeof(sem_t));
	b_reanudar_corto_plazo = malloc(sizeof(sem_t));
	b_exec_libre = malloc(sizeof(sem_t));

	sem_init(GRADO_MULTIPROGRAMACION, 0, valores_config->grado_multiprogramacion);

	sem_init(b_reanudar_largo_plazo, 0, 0);
	sem_init(sem_agregar_a_estado, 0, 0);
	sem_init(b_reanudar_corto_plazo, 0, 0);
	sem_init(b_exec_libre, 0, 1);
	sem_init(b_largo_plazo_exit, 0, 0);
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

void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado) // Añade un proceso a la cola New //MONITOR DE QUEUE_PUSH
{
	pthread_mutex_lock(cola_estado->mutex_estado);
	queue_push(cola_estado->estado, pcb);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	sem_post(cola_estado->contador);
}

t_pcb *sacar_procesos_cola(colaEstado *cola_estado)
{
	t_pcb *pcb = malloc(sizeof(pcb));
	sem_wait(cola_estado->contador);
	pthread_mutex_lock(cola_estado->mutex_estado);
	pcb = queue_pop(cola_estado->estado);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	return pcb;
}

void *levantar_CPU_Interrupt(void *ptr)
{
	t_args *datosConexion = malloc(sizeof(t_args));
	datosConexion = (t_args *)ptr;
	socket_cpu_interrupt = levantarCliente(logger, "CPU", datosConexion->ip, datosConexion->puerto, "KERNEL SE CONECTO A CPU INTERRUPT");
	free(datosConexion);
}

void *levantar_CPU_Dispatch(void *ptr)
{
	t_args *datosConexion = malloc(sizeof(t_args));
	datosConexion = (t_args *)ptr;
	socket_cpu_dispatch = levantarCliente(logger, "CPU", datosConexion->ip, datosConexion->puerto, "KERNEL SE CONECTO A CPU DISPATCH");

	free(datosConexion);

	while (1)
	{

		op_code otro_codigo = recibir_operacion(socket_cpu_dispatch);

		if (otro_codigo == FINALIZAR_PROCESO)
		{
			log_info(logger, "Recibi Finalizar proceso");

			tipo_buffer *buffer_cpu = recibir_buffer(socket_cpu_dispatch); // recibo buffer

			t_cde *cde = leer_cde(buffer_cpu);

			log_info(logger, "Se finalizo el proceso: %d", cde->pid);

			sem_post(b_largo_plazo_exit); // otro hilo de largo plazo manda el proceso en exec a exit y aumenta el grado de multiprogramacion => tambien llama a finalizar_proceso(PID)
		}
		else
		{
			// FALTA VER COMO MOSTRAMOS EL MOTIVO POR EL QUE HA FINALIZADO EL PROCESO
			// log_info(logger, "No se pudo finalizar el proceso %d", PID);
		}
	}
}

void atender_interrupciones()
{
	tipo_buffer *buffer_desde_cpu = crear_buffer();
	op_code motivo_interrupcion = leer_buffer_enteroUint32(buffer_desde_cpu);

	switch (motivo_interrupcion)
	{
	case FIN_DE_QUANTUM:
		t_cde *cde_proceso_interrumpido = leer_cde(buffer_desde_cpu);
		t_pcb *proceso_interrumpido = sacar_procesos_cola(cola_exec_global);
		proceso_interrumpido->cde = cde_proceso_interrumpido;
		agregar_a_estado(proceso_interrumpido, cola_ready_global);
		free(proceso_interrumpido);
		break;
	case BLOQUEADO_POR_IO:
		cde_proceso_interrumpido = leer_cde(buffer_desde_cpu);
		proceso_interrumpido = sacar_procesos_cola(cola_exec_global);
		proceso_interrumpido->cde = cde_proceso_interrumpido;
		agregar_a_estado(proceso_interrumpido, cola_bloqueado_global);
		free(proceso_interrumpido);
		break;
	case FINALIZAR_PROCESO:

	default:
		log_error(logger, "Motivo de interrupcion incorrecto");
		break;
	}
}

void recibir_orden_interfaces_de_cpu()
{
	op_code operacion_desde_cpu_dispatch = recibir_operacion(socket_cpu_dispatch);
	switch (operacion_desde_cpu_dispatch)
	{
	case SOLICITUD_INTERFAZ_GENERICA: // ocurre cuando se va a ejecutar la instruccion IO_GEN_SLEEP

		sem_post(cola_bloqueado_global->contador);

		tipo_buffer *buffer_dispatch = recibir_buffer(socket_cpu_dispatch);
		t_tipoDeInstruccion instruccion_a_ejecutar = leer_buffer_enteroUint32(buffer_dispatch);
		uint32_t unidades_trabajo = leer_buffer_enteroUint32(buffer_dispatch);
		nombre_IO = leer_buffer_string(buffer_dispatch); // recibo la interfaz que debe hacer la operacion pedida por la cpu

		t_pcb *proceso_quitado = sacar_procesos_cola(cola_exec_global);

		proceso_quitado->estado = BLOCKED;

		agregar_a_estado(proceso_quitado, cola_bloqueado_global);

		destruir_buffer(buffer_dispatch);

		tipo_buffer *buffer_interfaz = crear_buffer();

		if (list_find(lista_interfaces, interfaz_esta_conectada) == NULL)
		{
			interfaz_no_conectada(); // IMPLEMENTAR
		}
		else
		{
			interfaz_conectada(buffer_interfaz, unidades_trabajo, instruccion_a_ejecutar);
		}
		break;
	case SOLICITUD_INTERFAZ_STDIN:
		break;
	case SOLICITUD_INTERFAZ_STDOUT:
		break;
	case SOLICITUD_INTERFAZ_DIALFS:
		break;
	default:
		log_error(logger, "No comprendo la solicitud enviada");
		break;
	}
}

void interfaz_no_conectada() // IMPLEMENTAR
{
	log_error(logger, "La interfaz %s no se encuentra conectada", nombre_IO);
	// enviar_proceso_a_exit();
	//  enviar proceso que pidio la ejecucion de la instruccion a exit
}

void interfaz_conectada(tipo_buffer *buffer_interfaz, int unidades_trabajo, t_tipoDeInstruccion instruccion_a_ejecutar)
{
	log_info(logger, "La interfaz %s esta conectada", nombre_IO);
	enviar_cod_enum(socket_interfaz, CONSULTAR_DISPONIBILDAD);
	op_code operacion_io = recibir_operacion(socket_interfaz);
	if (operacion_io == ESTOY_LIBRE)
	{
		agregar_buffer_para_enterosUint32(buffer_interfaz, instruccion_a_ejecutar);
		agregar_buffer_para_enterosUint32(buffer_interfaz, unidades_trabajo);
		enviar_buffer(buffer_interfaz, socket_io);
		operacion_io = recibir_operacion(socket_interfaz);
		if (operacion_io == CONCLUI_OPERACION)
			enviar_cod_enum(socket_cpu_dispatch, EJECUCION_IO_GEN_SLEEP_EXITOSA);
	}
	else
	{
		operacion_io = recibir_operacion(socket_interfaz);
		if (operacion_io == CONCLUI_OPERACION)
		{
			interfaz_conectada(buffer_interfaz, unidades_trabajo, instruccion_a_ejecutar);
		}
	}
}