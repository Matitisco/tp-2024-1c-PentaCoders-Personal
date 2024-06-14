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
char *recurso_recibido;
int socket_interfaz;
int disp_io;
t_pcb *proceso_interrumpido;
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

	char **lista_recursos = config_get_array_value(configuracion->config, "RECURSOS");
	char **instancias_recursos_str = config_get_array_value(configuracion->config, "INSTANCIAS_RECURSOS");

	int tamanio = 0;
	while (instancias_recursos_str[tamanio] != NULL)
	{
		tamanio++;
	}

	configuracion->recursos = malloc(tamanio * sizeof(t_recurso *));

	for (int i = 0; i < tamanio; i++)
	{ // Con este for armo el struct de los recursos
		configuracion->recursos[i] = (t_recurso *)malloc(sizeof(t_recurso));
		configuracion->recursos[i]->nombre = strdup(lista_recursos[i]);
		int instancias = atoi(instancias_recursos_str[i]);
		sem_init(&(configuracion->recursos[i]->instancias), 0, instancias);
	}
	// Libero los arrays
	for (int i = 0; instancias_recursos_str[i] != NULL; i++)
	{
		free(instancias_recursos_str[i]);
		free(lista_recursos[i]);
	}
	free(instancias_recursos_str);
	free(lista_recursos);

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

			tipo_buffer *buffer_cpu_fin = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu_fin);
			finalizar_proceso(cde_interrumpido->pid, SUCCESS);

			sem_post(b_largo_plazo_exit);
			sem_post(b_reanudar_largo_plazo);
			sem_post(b_reanudar_corto_plazo);
			destruir_buffer(buffer_cpu_fin);
			break;

		case FIN_DE_QUANTUM:

			buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu);
			log_info(logger, "Desalojo proceso por fin de Quantum: %d", cde_interrumpido->pid);
			pthread_cancel(hiloQuantum); // reseteo hilo de quantum
			sem_post(b_transicion_exec_ready);
			// sem_post(b_reanudar_corto_plazo);
			break;

		case INSTRUCCION_INTERFAZ:

			tipo_buffer *buffer_cde = recibir_buffer(socket_cpu_dispatch);
			buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cde);

			sem_post(b_transicion_exec_blocked);
			sem_post(b_reanudar_largo_plazo);

			recibir_orden_interfaces_de_cpu(cde_interrumpido->pid, buffer_cpu);
			break;

		case WAIT_RECURSO: // asignamos un recurso a un proceso

			// recibir_recurso(); // Para usarlo en wait y signal LUEGO VEMOS SI ANDA WAIT
			tipo_buffer *buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu);

			recurso_recibido = leer_buffer_string(buffer_cpu);
			destruir_buffer(buffer_cpu);

			sem_post(b_transicion_exec_blocked);
			int posicion;
			int recurso = existe_recurso(&posicion);
			if (recurso == 0) // encontro al recurso y existe
			{
				wait_instancia_recurso(posicion); // el SO pierde uno del recurso llamado

				int valor_instancias;
				sem_getvalue(&(valores_config->recursos[posicion]->instancias), &valor_instancias);
				log_info(logger, "Recurso: %s Instancias Restantes: %d", valores_config->recursos[posicion]->nombre, valor_instancias);
				sem_post(b_transicion_blocked_ready);
				sem_post(b_reanudar_largo_plazo);
				sem_post(b_reanudar_corto_plazo);
			}
			/*else
			{
				log_info(logger, "El Recurso Pedido No Existe En El Sistema");
				finalizar_proceso(cde_interrumpido->pid, INVALID_RESOURCE);
				sem_post(b_largo_plazo_exit);
				sem_post(b_reanudar_largo_plazo);
				sem_post(b_reanudar_corto_plazo);
			}*/
			break;
		case SIGNAL_RECURSO: // un proceso libera un recurso

			buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu);
			recurso_recibido = leer_buffer_string(buffer_cpu);
			destruir_buffer(buffer_cpu);

			sem_post(b_transicion_exec_blocked);
			posicion;
			recurso = existe_recurso(&posicion);
			if (recurso == 0) // encontro al recurso y existe
			{
				signal_instancia_recurso(posicion);
				
				// sem_wait(recurso_buscado->instancias);

				int valor_instancias;

				sem_getvalue(&(valores_config->recursos[posicion]->instancias), &valor_instancias);
				log_info(logger, "Recurso: %s Instancias Restantes: %d", valores_config->recursos[posicion]->nombre, valor_instancias);
				sem_post(b_transicion_blocked_ready);
				sem_post(b_reanudar_largo_plazo);
				sem_post(b_reanudar_corto_plazo);
			}
		
			break;
		default:
			log_warning(logger, "La operacion enviada por CPU Dispatch no la Puedo ejecutar");
			break;
		}
	}
}

_Bool ya_tiene_instancias_del_recurso(t_recurso *recurso_proceso)
{
	if (strcmp(recurso_recibido, recurso_proceso->nombre) == 0)
	{
		return 1;
	}
	return 0;
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

		instruccion_a_ejecutar = leer_buffer_enteroUint32(buffer_con_instruccion); // IO_GEN_SLEEP
		uint32_t unidades_trabajo = leer_buffer_enteroUint32(buffer_con_instruccion);
		nombre_IO = leer_buffer_string(buffer_con_instruccion);

		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_conectada);

		if (informacion_interfaz == NULL)
		{
			interfaz_no_conectada(pid);
		}
		else
		{
			log_info(logger, "La interfaz %s esta conectada", nombre_IO);
			interfaz_conectada_generica(unidades_trabajo, instruccion_a_ejecutar, informacion_interfaz->cliente_io, pid);
			destruir_buffer(buffer_con_instruccion);
		}
		break;

	case SOLICITUD_INTERFAZ_STDIN:

		instruccion_a_ejecutar = leer_buffer_enteroUint32(buffer_con_instruccion); // IO_STDIN_READ
		tamanioRegistro = leer_buffer_enteroUint32(buffer_con_instruccion);
		direccion_fisica = leer_buffer_enteroUint32(buffer_con_instruccion);
		nombre_IO = leer_buffer_string(buffer_con_instruccion);

		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_conectada);

		if (informacion_interfaz == NULL)
		{
			interfaz_no_conectada(pid);
		}
		else
		{
			log_info(logger, "La interfaz %s esta conectada", nombre_IO);
			interfaz_conectada_stdin(instruccion_a_ejecutar, tamanioRegistro, direccion_fisica, informacion_interfaz->cliente_io, pid);
			destruir_buffer(buffer_con_instruccion);
		}
		break;

	case SOLICITUD_INTERFAZ_STDOUT:

		instruccion_a_ejecutar = leer_buffer_enteroUint32(buffer_con_instruccion); // IO_STDOUT_WRITE
		tamanioRegistro = leer_buffer_enteroUint32(buffer_con_instruccion);
		direccion_fisica = leer_buffer_enteroUint32(buffer_con_instruccion);
		nombre_IO = leer_buffer_string(buffer_con_instruccion);

		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_conectada);

		if (informacion_interfaz == NULL)
		{
			interfaz_no_conectada(pid);
		}
		else
		{
			log_info(logger, "La interfaz %s esta conectada", nombre_IO);
			interfaz_conectada_stdout(instruccion_a_ejecutar, tamanioRegistro, direccion_fisica, informacion_interfaz->cliente_io, pid);
			destruir_buffer(buffer_con_instruccion);
		}
		break;

	case SOLICITUD_INTERFAZ_DIALFS:
		break;

	default:
		log_error(logger, "ERROR - Solicitud Interfaz Enviada Por CPU");
		break;
	}
}

void interfaz_no_conectada(int pid)
{
	log_error(logger, "La interfaz %s no se encuentra conectada", nombre_IO);
	finalizar_proceso(pid, INVALID_RESOURCE);
	sem_post(b_largo_plazo_exit);
	sem_post(b_reanudar_largo_plazo);
	sem_post(b_reanudar_corto_plazo);
}

void interfaz_conectada_generica(int unidades_trabajo, t_tipoDeInstruccion instruccion_a_ejecutar, int socket_io, int pid)
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
			// fijarnos si hay proceso bloqueados en la lista de interfaz y enviarlos
			sem_post(b_reanudar_largo_plazo);
			sem_post(b_reanudar_corto_plazo);
		}
	}
	else
	{
		// mandar a bloquear el proceso a la lista de bloqueados de la interfaz
	}
}

void interfaz_conectada_stdin(t_tipoDeInstruccion instruccion_a_ejecutar, int tamanio_reg, int dir_fisica, int socket_io, int pid)
{
	enviar_cod_enum(socket_io, CONSULTAR_DISPONIBILDAD);

	op_code operacion_io = recibir_operacion(socket_io);
	tipo_buffer *buffer_interfaz = crear_buffer();

	if (operacion_io == ESTOY_LIBRE)
	{
		agregar_buffer_para_enterosUint32(buffer_interfaz, instruccion_a_ejecutar);
		agregar_buffer_para_enterosUint32(buffer_interfaz, tamanio_reg);
		agregar_buffer_para_enterosUint32(buffer_interfaz, dir_fisica);
		agregar_buffer_para_enterosUint32(buffer_interfaz, pid);
		enviar_buffer(buffer_interfaz, socket_io);
		operacion_io = recibir_operacion(socket_io);

		if (operacion_io == CONCLUI_OPERACION)
		{
			sem_post(b_transicion_blocked_ready);
			// fijarnos si hay proceso bloqueados en la lista de interfaz y enviarlos
			sem_post(b_reanudar_largo_plazo);
			sem_post(b_reanudar_corto_plazo);
		}
	}
	else
	{
		// mandar a bloquear el proceso a la lista de bloqueados de la interfaz
	}
}

void interfaz_conectada_stdout(t_tipoDeInstruccion instruccion_a_ejecutar, int tamanio_reg, int dir_fisica, int socket_io, int pid)
{
	enviar_cod_enum(socket_io, CONSULTAR_DISPONIBILDAD);

	op_code operacion_io = recibir_operacion(socket_io);
	tipo_buffer *buffer_interfaz = crear_buffer();

	if (operacion_io == ESTOY_LIBRE)
	{
		agregar_buffer_para_enterosUint32(buffer_interfaz, instruccion_a_ejecutar);
		agregar_buffer_para_enterosUint32(buffer_interfaz, tamanio_reg);
		agregar_buffer_para_enterosUint32(buffer_interfaz, dir_fisica);
		agregar_buffer_para_enterosUint32(buffer_interfaz, pid);
		enviar_buffer(buffer_interfaz, socket_io);
		operacion_io = recibir_operacion(socket_io);

		if (operacion_io == CONCLUI_OPERACION)
		{
			sem_post(b_transicion_blocked_ready);
			// fijarnos si hay proceso bloqueados en la lista de interfaz y enviarlos
			sem_post(b_reanudar_largo_plazo);
			sem_post(b_reanudar_corto_plazo);
		}
	}
	else
	{
		// mandar a bloquear el proceso a la lista de bloqueados de la interfaz
	}
}

int existe_recurso(int *posicion)
{
	char *recurso_encontrado = buscar_recurso(recurso_recibido, posicion);

	if (recurso_encontrado == NULL)
	{
		log_info(logger, "El Recurso Pedido No Existe En El Sistema \n Envio proceso a EXIT");

		finalizar_proceso(cde_interrumpido->pid, INVALID_RESOURCE);
		sem_post(b_largo_plazo_exit);
		sem_post(b_reanudar_largo_plazo);
		sem_post(b_reanudar_corto_plazo);
		return -1;
	}
	return 0;
}

char *buscar_recurso(char *recurso, int *posicion)
{
	char *recurso_encontrado = NULL;

	int i = 0;
	// while (valores_config->listaRecursos[i]!=NULL) FUNKA
	while (valores_config->recursos[i]->nombre != NULL)
	{
		if (strcmp(recurso, valores_config->recursos[i]->nombre) == 0)
		{
			recurso_encontrado = recurso;
			*posicion = i;
			break;
		}
		i++;
	}

	return recurso_encontrado;
}

void wait_instancia_recurso(int i)
{
	int valor;
	sem_getvalue(&(valores_config->recursos[i]->instancias), &valor);
	proceso_interrumpido = buscarProceso(cde_interrumpido->pid);
	if (valor > 0) // Si hay instancias del recurso, puedo restar y asignar a un proceso
	{
		sem_wait(&(valores_config->recursos[i]->instancias)); // le doy una instancia al proceso, y le quito una a la lista de recursos que tiene el SO
		// De aca para abajo asigna al proceso los recursos retenidos

		t_recurso *recurso_buscado = list_find(proceso_interrumpido->recursosAsignados, ya_tiene_instancias_del_recurso);
		if (recurso_buscado == NULL) // el caso de que el proceso no contaba con el recurso ya cargado
		{
			log_info(logger, "Recurso: <%s> No estaba cargado - PID: <%d>", recurso_recibido, proceso_interrumpido->cde->pid);
			t_recurso *recurso_asignado = malloc(sizeof(t_recurso));
			recurso_asignado->nombre = recurso_recibido;
			recurso_asignado->instancias = malloc(sizeof(sem_t));
			recurso_asignado->cola_bloqueados = constructorColaEstado(recurso_recibido);
			sem_init(&(recurso_asignado->instancias), 0, 1);
			list_add(proceso_interrumpido->recursosAsignados, recurso_asignado);
			log_info(logger, "Recurso: <%s> Cargado - PID: <%d>", recurso_asignado->nombre, proceso_interrumpido->cde->pid);
		}
		else // ya tiene el recurso cargado el proceso
		{
			log_info(logger, "Recurso: <%s> Estaba cargado - PID: <%d>", recurso_recibido, proceso_interrumpido->cde->pid);
			sem_post(&recurso_buscado->instancias);
			log_info(logger, "Recurso: <%s> Cargado - PID: <%d>", recurso_recibido, proceso_interrumpido->cde->pid);
		}
	}
	else
	{
		log_info(logger, "NO HAY INSTANCIAS DISPONIBLES DEL RECURSO PEDIDO");
		log_info(logger, "TRANSICION EXEC A BLOCKED");
		// mandar proceso a cola de bloqueados correspondiente al recurso
		

		//agregar_a_estado(proceso_interrumpido, &recurso_buscado->cola_bloqueados);
		sem_post(b_transicion_exec_blocked);
		//log_info(logger, "Se bloqueo el proceso: %d por el recurso %s", proceso_interrumpido->cde->pid, recurso_recibido);
	}
}

/*

void wait_instancia_recurso(int i, char* recurso_recibido)
{
	int valor;
	sem_getvalue(&(valores_config->recursos[i]->instancias), &valor);
	if (valor > 0)//Si hay instancias, puedo restar
	{
		sem_wait(&(valores_config->recursos[i]->instancias)); // le doy una instancia al proceso, y le quito una a la lista de recursos que tiene el SO
		//De aca para abajo asigna al proceso los recursos retenidos
		
		t_pcb *proceso = buscarProceso(cde_interrumpido->pid);
		t_recurso *recurso_buscado = list_find(proceso->recursosAsignados, ya_tiene_instancias_del_recurso);
		if (recurso_buscado == NULL) // el caso de que el proceso no contaba con el recurso ya cargado
		{
			t_recurso *recurso_asignado = malloc(sizeof(t_recurso));
			recurso_asignado->nombre = recurso_recibido;
			recurso_asignado->instancias = malloc(sizeof(sem_t));
			sem_init(recurso_asignado->instancias, 0, 0);
			sem_post(recurso_asignado->instancias);
			list_add(proceso->recursosAsignados, recurso_asignado);
		}
		else // ya tiene el recurso cargado el proceso
		{
			sem_post(recurso_buscado->instancias);
		}
	}
	else
	{
		log_info(logger, "NO HAY INSTANCIAS DISPONIBLES DEL RECURSO PEDIDO");
		log_info(logger, "TRANSICION EXEC A BLOCKED");
		// mandar proceso a cola de bloqueados correspondiente al recurso
		sem_post(b_transicion_exec_blocked); //!!!!!!!!!!!
	}
}
*/

void signal_instancia_recurso(int i)
{
	sem_post(&(valores_config->recursos[i]->instancias));
	
	//aca tengo que ver si tengo un proceso bloqueado por un recurso, si ESTE signal se esta haciendo a ESE recurso
	//bloqueante, verifico si ya se puede desbloquear el proceso 
}

/**

Manejo de Recursos
1) 	Los recursos del sistema vendrán indicados por medio del archivo de configuración, 
	donde se encontrarán 2 variables con la información inicial de los mismos:
	La primera llamada RECURSOS, la cual listará los nombres de los recursos disponibles en el sistema.

	La segunda llamada INSTANCIAS_RECURSOS será la cantidad de instancias de cada recurso del sistema, 
	y estarán ordenadas de acuerdo a la lista anterior (ver ejemplo)

RECURSOS=[RA,RB,RC]
INSTANCIAS_RECURSOS=[1,2,1]

2)	A la hora de recibir de la CPU un Contexto de Ejecución desalojado por WAIT, el Kernel deberá verificar
	primero que exista el recurso solicitado y en caso de que exista restarle 1 a la cantidad de instancias 
	del mismo. En caso de que el número sea estrictamente menor a 0, el proceso que realizó WAIT se bloqueará 
	en la cola de bloqueados correspondiente al recurso.

3)	A la hora de recibir de la CPU un Contexto de Ejecución desalojado por SIGNAL, el Kernel deberá verificar 
	primero que exista el recurso solicitado, luego sumarle 1 a la cantidad de instancias del mismo. En caso de 
	que corresponda, desbloquea al primer proceso de la cola de bloqueados de ese recurso. Una vez hecho esto, 
	se devuelve la ejecución al proceso que peticiona el SIGNAL.
	Para las operaciones de WAIT y SIGNAL donde no se cumpla que el recurso exista, se deberá enviar el proceso a EXIT.

consumidor:
WAIT RB
WAIT RA
WAIT RB
WAIT RB
WAIT RB
EXIT

WAIT (Recurso): Esta instrucción solicita al Kernel que se asigne una instancia del recurso indicado por parámetro.

productor:
SIGNAL RC
SIGNAL RC
EXIT

SIGNAL (Recurso): Esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro.



Diagrama de estados
El kernel utilizará un diagrama de 5 estados para la planificación de los procesos. Dentro del estado
BLOCK, se tendrán múltiples colas, las cuales pueden ser correspondientes a operaciones de I/O,
teniendo una por cada interfaz de I/O conectada y correspondientes a los recursos que administra el
Kernel, teniendo una por cada recur


Eliminación de Procesos
Ante la llegada de un proceso al estado de EXIT (ya sea por solicitud de la CPU, por un error, o por
ejecución desde la consola del Kernel), el Kernel deberá solicitar a la memoria que libere todas las
estructuras asociadas al proceso y marque como libre todo el espacio que este ocupaba.
En caso de que el proceso se encuentre ejecutando en CPU, se deberá enviar una señal de
interrupción a través de la conexión de interrupt con el mismo y aguardar a que éste retorne el
Contexto de Ejecución antes de iniciar la liberación de recursos.


Finalizar proceso: Se encargará de finalizar un proceso que se encuentre dentro del sistema.
Este mensaje se encargará de realizar las mismas operaciones como si el proceso llegara a
EXIT por sus caminos habituales (deberá liberar recursos, archivos y memoria).

*/