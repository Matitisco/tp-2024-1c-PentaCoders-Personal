#include "../include/kernel.h"

// COLAS
colaEstado *cola_new_global;
colaEstado *cola_ready_global;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;
// INTEGERS
int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;
int socket_interfaz;
int disp_io;
int habilitar_planificadores;
int QUANTUM;
// Contadores Semaforos
sem_t *GRADO_MULTIPROGRAMACION;
// Semaforos de binarios
sem_t *binario_menu_lp;
sem_t *b_largo_plazo_exit;
sem_t *b_transicion_exec_ready;
sem_t *b_transicion_exec_blocked;
sem_t *b_exec_libre;
sem_t *sem_quantum;
sem_t *sem_agregar_a_estado;
sem_t *b_reanudar_largo_plazo;
sem_t *b_reanudar_corto_plazo;
sem_t *b_transicion_blocked_ready;
extern sem_t *sem_kernel_io_generica;
// HILOS
pthread_t hiloMEMORIA;
pthread_t hiloIO;
pthread_t hiloLargoPlazo;
pthread_t hiloCortoPlazo;
pthread_t hiloCPUINT;
pthread_t hiloCPUDS;
pthread_t hiloConsola;
pthread_t largo_plazo_exit;
pthread_t t_transicion_exec_ready;
pthread_t t_transicion_exec_blocked;
pthread_t t_transicion_blocked_ready;
pthread_t hiloQuantum;
t_cde *cde_interrumpido;
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

	iniciar_kernel();

	pthread_join(hiloMEMORIA, NULL);
	pthread_join(hiloLargoPlazo, NULL);
	pthread_join(hiloCortoPlazo, NULL);
	pthread_join(hiloCPUDS, NULL);
	pthread_join(hiloCPUINT, NULL);
	pthread_join(hiloIO, NULL);
	pthread_join(hiloConsola, NULL);
	pthread_join(largo_plazo_exit, NULL);
	pthread_join(t_transicion_exec_ready, NULL);
	pthread_join(hiloQuantum, NULL);
}

void iniciar_kernel()
{
	inicializarEstados();
	logger = iniciar_logger("kernel.log", "KERNEL");
	nombre_IO = string_new();
	lista_interfaces = list_create();
	valores_config = inicializar_config_kernel();
	QUANTUM = valores_config->quantum;

	iniciar_semaforos();
	levantar_servidores();
	crear_hilos();
}

void crear_hilos()
{
	pthread_create(&hiloMEMORIA, NULL, conexionAMemoria, NULL);
	pthread_create(&hiloIO, NULL, levantarIO, NULL);
	pthread_create(&hiloCPUDS, NULL, levantar_CPU_Dispatch, NULL);
	pthread_create(&hiloCPUINT, NULL, levantar_CPU_Interrupt, NULL);
	pthread_create(&hiloLargoPlazo, NULL, largo_plazo, NULL);
	pthread_create(&hiloCortoPlazo, NULL, corto_plazo, NULL);
	pthread_create(&hiloConsola, NULL, iniciar_consola_interactiva, NULL);
	pthread_create(&largo_plazo_exit, NULL, transicion_exit_largo_plazo, NULL);
	pthread_create(&t_transicion_exec_blocked, NULL, transicion_exec_blocked, NULL);
	pthread_create(&t_transicion_exec_ready, NULL, transicion_exec_ready, NULL);
	pthread_create(&t_transicion_blocked_ready, NULL, transicion_blocked_ready, NULL);
}

void levantar_servidores()
{
	servidor_para_io = iniciar_servidor(logger, "Kernel", valores_config->ip_memoria, valores_config->puerto_escucha);
}

int llego_proceso()
{
	int *sem_value_q = malloc(sizeof(int));
	sem_getvalue(cola_exec_global->contador, sem_value_q); // 0 o 1
	log_info(logger, "VAlor sem: %d", *sem_value_q);
	return *sem_value_q;
}

void *levantarIO()
{
	while (1)
	{
		disp_io = esperar_cliente(logger, "Kernel", "Interfaz IO", servidor_para_io); // se conecta una io al kernel
		op_code operacion = recibir_operacion(disp_io);
		if (operacion == SOLICITUD_CONEXION_IO)
		{
			tipo_buffer *buffer_io = recibir_buffer(disp_io);

			int cod_io = leer_buffer_enteroUint32(buffer_io);
			nombre_IO = leer_buffer_string(buffer_io);
			char *tipo_io = obtener_interfaz(cod_io);

			t_infoIO *infoIO = malloc(sizeof(t_infoIO));
			infoIO->cliente_io = disp_io;
			infoIO->nombre_io = nombre_IO;

			if (list_find(lista_interfaces, interfaz_esta_conectada) != NULL)
			{
				log_info(logger, "La Interfaz %s ya esta conectada", infoIO->nombre_io);
				enviar_cod_enum(disp_io, ESTABA_CONECTADO);
			}
			else
			{
				enviar_cod_enum(disp_io, NO_ESTABA_CONECTADO);
				log_info(logger, "La Interfaz %s no estaba conectada", infoIO->nombre_io);
				list_add(lista_interfaces, infoIO);
				log_info(logger, "Se conecto una interfaz del tipo: %s, y de nombre: %s", tipo_io, infoIO->nombre_io);
			}
		}
		else
		{
			log_info(logger, "No entiendo la operacion enviada");
		}
	}
	return NULL;
}

_Bool interfaz_esta_conectada(t_infoIO *interfaz)
{
	return strcmp(interfaz->nombre_io, nombre_IO) == 0;
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

void *conexionAMemoria()
{
	socket_memoria = levantarCliente(logger, "MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria);
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
	cola_ready_global->contador = malloc(sizeof(sem_t));
	sem_init(cola_ready_global->contador, 0, 0);
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
	b_transicion_exec_ready = malloc(sizeof(sem_t));
	b_transicion_exec_blocked = malloc(sizeof(sem_t));
	b_transicion_blocked_ready = malloc(sizeof(sem_t));
	sem_quantum = malloc(sizeof(sem_t));

	sem_init(GRADO_MULTIPROGRAMACION, 0, valores_config->grado_multiprogramacion);
	sem_init(b_reanudar_largo_plazo, 0, 0);
	sem_init(sem_agregar_a_estado, 0, 0);
	sem_init(b_reanudar_corto_plazo, 0, 0);
	sem_init(b_exec_libre, 0, 1);
	sem_init(b_largo_plazo_exit, 0, 0);
	sem_init(b_transicion_exec_ready, 0, 0);
	sem_init(sem_quantum, 0, 0);
	sem_init(b_transicion_exec_blocked, 0, 0);
	sem_init(b_transicion_blocked_ready, 0, 0);
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
	configuracion->listaRecursos = config_get_array_value(configuracion->config, "RECURSOS");
	configuracion->instanciasRecursos = config_get_array_value(configuracion->config, "INSTANCIAS_RECURSOS");

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

void *levantar_CPU_Interrupt()
{
	socket_cpu_interrupt = levantarCliente(logger, "CPU", valores_config->ip_cpu, valores_config->puerto_cpu_interrupt);
}

void *levantar_CPU_Dispatch()
{
	socket_cpu_dispatch = levantarCliente(logger, "CPU", valores_config->ip_cpu, valores_config->puerto_cpu_dispatch);

	while (1)
	{
		op_code cod = recibir_operacion(socket_cpu_dispatch);
		tipo_buffer *buffer_cpu;

		switch (cod)
		{
		case FINALIZAR_PROCESO:
			// debemos llamar a la funcion finalizar_proceso(int pid) de operaciones para esto y que luego siga el codigo normal
			buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu);
			// pthread_cancel(hiloQuantum); VER QUE ONDA para rr y vrr
			log_info(logger, "Se finalizo el proceso: %d", cde_interrumpido->pid);
			sem_post(b_largo_plazo_exit);
			sem_post(b_reanudar_largo_plazo);
			sem_post(b_reanudar_corto_plazo);
			break;

		case FIN_DE_QUANTUM:

			buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu);
			log_info(logger, "Desalojo proceso por fin de Quantum: %d", cde_interrumpido->pid);
			pthread_cancel(hiloQuantum); // reseteo hilo de quantum
			sem_post(b_transicion_exec_ready);
			// sem_post(b_reanudar_corto_plazo);
			break;

		case INSTRUCCION_INTERFAZ: // recibimos de cpu ciclo de instruccion // ESTO YA ANDA

			tipo_buffer *buffer_cde = recibir_buffer(socket_cpu_dispatch); // recibimos CDE
			buffer_cpu = recibir_buffer(socket_cpu_dispatch);			   // recibimos datos de la instruccion
			cde_interrumpido = leer_cde(buffer_cde);
			sem_post(b_transicion_exec_blocked); // bloqueamos el proceso interrumpido
			sem_post(b_reanudar_largo_plazo);
			recibir_orden_interfaces_de_cpu(cde_interrumpido->pid, buffer_cpu);
			break;

		default:
			log_warning(logger, "La operacion enviada por CPU Dispatch no la Puedo ejecutar");
			break;
		}
	}
}

void enviar_proceso_exec_a_exit()
{
	t_pcb *pcb_quitado = sacar_procesos_cola(cola_exec_global);
	agregar_a_estado(pcb_quitado, cola_exit_global);
}

void recibir_orden_interfaces_de_cpu(int pid, tipo_buffer *buffer_con_instruccion)
{
	op_code operacion_desde_cpu_dispatch = leer_buffer_enteroUint32(buffer_con_instruccion);
	t_infoIO *informacion_interfaz;
	t_tipoDeInstruccion instruccion_a_ejecutar;
	uint32_t tamanioRegistro;
	uint32_t direccion_fisica;
	switch (operacion_desde_cpu_dispatch)
	{
	case SOLICITUD_INTERFAZ_GENERICA:

		instruccion_a_ejecutar = leer_buffer_enteroUint32(buffer_con_instruccion);
		uint32_t unidades_trabajo = leer_buffer_enteroUint32(buffer_con_instruccion);
		nombre_IO = leer_buffer_string(buffer_con_instruccion);

		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_conectada);

		if (informacion_interfaz == NULL)
		{
			interfaz_no_conectada();
		}
		else
		{
			interfaz_conectada_generica(unidades_trabajo, instruccion_a_ejecutar, informacion_interfaz->cliente_io, pid);
			destruir_buffer(buffer_con_instruccion);
		}

		break;
	case SOLICITUD_INTERFAZ_STDIN:
		instruccion_a_ejecutar = leer_buffer_enteroUint32(buffer_con_instruccion); // IO_STDIN_READ
		tamanioRegistro = leer_buffer_enteroUint32(buffer_con_instruccion);
		direccion_fisica = leer_buffer_enteroUint32(buffer_con_instruccion);
		nombre_IO = leer_buffer_string(buffer_con_instruccion);

		// falta que reciba mas cosas

		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_conectada);

		if (informacion_interfaz == NULL)
		{
			interfaz_no_conectada();
		}
		else
		{
			interfaz_conectada_stdin(pid);
			destruir_buffer(buffer_con_instruccion);
		}

		break;
	case SOLICITUD_INTERFAZ_STDOUT:
		instruccion_a_ejecutar = leer_buffer_enteroUint32(buffer_con_instruccion); // IO_STDOUT_WRITE
		// falta que reciba mas cosas
		tamanioRegistro = leer_buffer_enteroUint32(buffer_con_instruccion);
		direccion_fisica = leer_buffer_enteroUint32(buffer_con_instruccion);
		nombre_IO = leer_buffer_string(buffer_con_instruccion);

		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_conectada);

		if (informacion_interfaz == NULL)
		{
			interfaz_no_conectada();
		}
		else
		{
			interfaz_conectada_stdout(pid);
			destruir_buffer(buffer_con_instruccion);
		}
		break;
	case SOLICITUD_INTERFAZ_DIALFS:
		break;
	default:
		log_error(logger, "No comprendo la solicitud enviada");
		break;
	}
}

void interfaz_no_conectada()
{
	log_error(logger, "La interfaz %s no se encuentra conectada", nombre_IO);
	enviar_proceso_exec_a_exit(); // talvez implementar con un hilo talvez
}

void interfaz_conectada_generica(int unidades_trabajo, t_tipoDeInstruccion instruccion_a_ejecutar, int socket_io, int pid)
{
	log_info(logger, "La interfaz %s esta conectada", nombre_IO);
	if (instruccion_a_ejecutar != IO_GEN_SLEEP)
	{
		enviar_proceso_exec_a_exit();
	}
	else
	{
		enviar_cod_enum(socket_io, CONSULTAR_DISPONIBILDAD);

		op_code operacion_io = recibir_operacion(socket_io);
		tipo_buffer *buffer_interfaz = crear_buffer();

		if (operacion_io == ESTOY_LIBRE)
		{
			agregar_buffer_para_enterosUint32(buffer_interfaz, instruccion_a_ejecutar);
			agregar_buffer_para_enterosUint32(buffer_interfaz, unidades_trabajo);
			agregar_buffer_para_enterosUint32(buffer_interfaz, pid);
			enviar_buffer(buffer_interfaz, socket_io);
			operacion_io = recibir_operacion(socket_io);
			if (operacion_io == CONCLUI_OPERACION)
			{
				sem_post(b_transicion_blocked_ready);
				sem_post(b_reanudar_largo_plazo);
				sem_post(b_reanudar_corto_plazo);
			}
		}
		else
		{
			operacion_io = recibir_operacion(socket_io);
			if (operacion_io == CONCLUI_OPERACION)
			{
				interfaz_conectada_generica(unidades_trabajo, instruccion_a_ejecutar, socket_io, pid);
			}
		}
	}
}

void interfaz_conectada_stdin(t_tipoDeInstruccion instruccion_a_ejecutar, int socket_io, int pid) // IMPLMEEMNTAR
{
	log_info(logger, "La interfaz %s esta conectada", nombre_IO);
	if (instruccion_a_ejecutar != IO_STDIN_READ)
	{
		enviar_proceso_exec_a_exit();
	}
	else
	{
		enviar_cod_enum(socket_io, CONSULTAR_DISPONIBILDAD);

		op_code operacion_io = recibir_operacion(socket_io);
		tipo_buffer *buffer_interfaz = crear_buffer();

		if (operacion_io == ESTOY_LIBRE)
		{
			/* agregar_buffer_para_enterosUint32(buffer_interfaz, instruccion_a_ejecutar);
			agregar_buffer_para_enterosUint32(buffer_interfaz, unidades_trabajo);
			agregar_buffer_para_enterosUint32(buffer_interfaz, pid);
			enviar_buffer(buffer_interfaz, socket_io);
			operacion_io = recibir_operacion(socket_io); */
			if (operacion_io == CONCLUI_OPERACION)
			{
				sem_post(b_transicion_blocked_ready);
			}
		}
		else
		{
			operacion_io = recibir_operacion(socket_io);
			if (operacion_io == CONCLUI_OPERACION)
			{
				interfaz_conectada_stdin(instruccion_a_ejecutar, socket_io, pid);
			}
		}
	}
}
void interfaz_conectada_stdout(int pid) // IMPLEMENTAR
{
}