#include "../include/kernel.h"

// COLAS
colaEstado *cola_new_global;
colaEstado *cola_ready_global;
colaEstado *cola_ready_plus;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;
// INTEGERS
int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;
char *nombre_recurso_recibido;
int socket_disp_io;
int habilitar_planificadores;
int QUANTUM;
int valorSem;
int valor_grado_a_modificar;
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
sem_t *b_reanudar_exit_largo;
sem_t *b_reanudar_exec_blocked;
sem_t *b_reanudar_exec_ready;
sem_t *b_reanudar_blocked_ready;

sem_t *manejo_grado;

sem_t *b_transicion_blocked_ready;
sem_t *bloquearReady;
sem_t *bloquearReadyPlus;

sem_t *cant_procesos_en_new;

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
pthread_t hiloMultiProg;

t_cde *cde_interrumpido;
t_list *lista_interfaces;
extern t_log *logger;
char *nombre_IO;
int servidor_para_io;
config_kernel *valores_config;

sem_t *b_detener_planificacion;

int interruptor_switch_readys;
sem_t *b_switch_readys;
sem_t *sem_finalizar_proceso;

sem_t *contador_readys;
char *comandos[] = {"EJECUTAR_SCRIPT", "INICIAR_PROCESO", "FINALIZAR_PROCESO", "DETENER_PLANIFICACION", "INICIAR_PLANIFICACION", "MULTIPROGRAMACION", "PROCESO_ESTADO", "HELP", NULL};
char *ip_local;

config_kernel *configuracion;

int main(int argc, char *argv[])
{
	ip_local = obtener_ip_local();
	printf("IP LOCAL: %s", ip_local);

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
	pthread_join(hiloMultiProg, NULL);
	free(ip_local);
	printf("Kernel finalizado");
}

void iniciar_kernel()
{
	habilitar_planificadores = 0;
	inicializarEstados();
	logger = iniciar_logger("kernel.log", "KERNEL");
	lista_interfaces = list_create();
	valores_config = inicializar_config_kernel();
	QUANTUM = valores_config->quantum;
	iniciar_semaforos();
	levantar_servidores();
	crear_hilos();
}

void crear_hilos()
{
	pthread_create(&hiloLargoPlazo, NULL, largo_plazo, NULL); // Done
	pthread_create(&hiloCortoPlazo, NULL, corto_plazo, NULL); // Done

	pthread_create(&hiloConsola, NULL, iniciar_consola_interactiva, NULL);
	pthread_create(&largo_plazo_exit, NULL, transicion_exit_largo_plazo, NULL);
	pthread_create(&t_transicion_exec_blocked, NULL, transicion_exec_blocked, NULL);
	pthread_create(&t_transicion_exec_ready, NULL, transicion_exec_ready, NULL);
	pthread_create(&t_transicion_blocked_ready, NULL, transicion_blocked_ready, NULL);

	pthread_create(&hiloMEMORIA, NULL, (void *)conexionAMemoria, NULL);
	pthread_create(&hiloCPUDS, NULL, (void *)levantar_CPU_Dispatch, NULL);
	pthread_create(&hiloCPUINT, NULL, (void *)levantar_CPU_Interrupt, NULL);
	pthread_create(&hiloIO, NULL, conectar_interfaces, NULL);
	pthread_create(&hiloMultiProg, NULL, (void *)manejarGrado, NULL);
}
void finalizar_hilos()
{
	pthread_cancel(hiloLargoPlazo);
	pthread_cancel(hiloCortoPlazo);
	pthread_cancel(hiloConsola);
	pthread_cancel(largo_plazo_exit);
	pthread_cancel(t_transicion_exec_blocked);
	pthread_cancel(t_transicion_exec_ready);
	pthread_cancel(t_transicion_blocked_ready);
	pthread_cancel(hiloMEMORIA);
	pthread_cancel(hiloIO);
	pthread_cancel(hiloCPUDS);
	pthread_cancel(hiloCPUINT);
}

void levantar_servidores()
{
	servidor_para_io = crear_servidor(valores_config->puerto_escucha);
}

void conexionAMemoria()
{
	socket_memoria = levantarCliente(logger, "MEMORIA", valores_config->ip_memoria, string_itoa(valores_config->puerto_memoria));
}

colaEstado *constructorColaEstado(char *nombre)
{
	colaEstado *cola_estado_generica;
	cola_estado_generica = malloc(sizeof(colaEstado));
	cola_estado_generica->nombreEstado = nombre;
	cola_estado_generica->estado = list_create();
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
	cola_ready_plus = constructorColaEstado("READY+");
	cola_exec_global = constructorColaEstado("EXEC");
	cola_bloqueado_global = constructorColaEstado("BLOCK");
	cola_exit_global = constructorColaEstado("EXIT");
}

void iniciar_semaforos()
{
	GRADO_MULTIPROGRAMACION = malloc(sizeof(sem_t));
	manejo_grado = malloc(sizeof(sem_t));
	binario_menu_lp = malloc(sizeof(sem_t));
	b_reanudar_largo_plazo = malloc(sizeof(sem_t));
	b_reanudar_corto_plazo = malloc(sizeof(sem_t));
	cant_procesos_en_new = malloc(sizeof(sem_t));

	b_reanudar_exit_largo = malloc(sizeof(sem_t));
	b_reanudar_exec_blocked = malloc(sizeof(sem_t));
	b_reanudar_exec_ready = malloc(sizeof(sem_t));
	b_reanudar_blocked_ready = malloc(sizeof(sem_t));

	sem_agregar_a_estado = malloc(sizeof(sem_t));
	b_largo_plazo_exit = malloc(sizeof(sem_t));
	b_exec_libre = malloc(sizeof(sem_t));
	b_transicion_exec_ready = malloc(sizeof(sem_t));
	b_transicion_exec_blocked = malloc(sizeof(sem_t));
	b_transicion_blocked_ready = malloc(sizeof(sem_t));
	sem_quantum = malloc(sizeof(sem_t));
	b_detener_planificacion = malloc(sizeof(sem_t));
	bloquearReady = malloc(sizeof(sem_t));
	bloquearReadyPlus = malloc(sizeof(sem_t));
	b_switch_readys = malloc(sizeof(sem_t));
	contador_readys = malloc(sizeof(sem_t));
	sem_finalizar_proceso = malloc(sizeof(sem_t));

	sem_init(GRADO_MULTIPROGRAMACION, 0, valores_config->grado_multiprogramacion);
	sem_init(cant_procesos_en_new, 0, 0);
	sem_init(b_reanudar_largo_plazo, 0, 0);
	sem_init(b_reanudar_corto_plazo, 0, 0);
	sem_init(manejo_grado, 0, 0);

	sem_init(b_reanudar_exit_largo, 0, 0);
	sem_init(b_reanudar_exec_blocked, 0, 0);
	sem_init(b_reanudar_exec_ready, 0, 0);
	sem_init(b_reanudar_blocked_ready, 0, 0);

	sem_init(sem_agregar_a_estado, 0, 0);
	sem_init(b_exec_libre, 0, 1);
	sem_init(b_largo_plazo_exit, 0, 0);
	sem_init(b_transicion_exec_ready, 0, 0);
	sem_init(sem_quantum, 0, 0);
	sem_init(b_transicion_exec_blocked, 0, 0);
	sem_init(b_transicion_blocked_ready, 0, 0);
	sem_init(b_detener_planificacion, 0, 0);
	sem_init(bloquearReady, 0, 0);
	sem_init(bloquearReadyPlus, 0, 0);
	sem_init(b_switch_readys, 0, 0);
	sem_init(contador_readys, 0, 0);
	sem_init(binario_menu_lp, 0, 0);
	sem_init(sem_finalizar_proceso, 0, 0);
}

config_kernel *inicializar_config_kernel()
{
	configuracion = (config_kernel *)malloc(sizeof(config_kernel));
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	char *ultimo_dir = basename(directorioActual);

	if (strcmp(ultimo_dir, "bin") == 0)
	{
		chdir("..");
		getcwd(directorioActual, sizeof(directorioActual));
	}
	strcat(directorioActual, "/kernel.config");

	configuracion->config = iniciar_config(directorioActual);

	configuracion->ip_memoria = config_get_string_value(configuracion->config, "IP_MEMORIA");
	configuracion->ip_cpu = config_get_string_value(configuracion->config, "IP_CPU");
	configuracion->puerto_memoria = config_get_int_value(configuracion->config, "PUERTO_MEMORIA");
	configuracion->puerto_escucha = config_get_int_value(configuracion->config, "PUERTO_ESCUCHA");
	configuracion->puerto_cpu_dispatch = config_get_int_value(configuracion->config, "PUERTO_CPU_DISPATCH");
	configuracion->puerto_cpu_interrupt = config_get_int_value(configuracion->config, "PUERTO_CPU_INTERRUPT");
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

	t_lista_recursos *recursos = list_create();

	for (int i = 0; i < tamanio; i++)
	{
		t_recurso *recurso = malloc(sizeof(t_recurso));
		recurso->nombre = strdup(lista_recursos[i]);

		recurso->instancias = malloc(sizeof(sem_t));
		int instancias = atoi(instancias_recursos_str[i]);
		sem_init(recurso->instancias, 0, instancias);

		recurso->cola_bloqueados = constructorColaEstado("BLOCK_RECURSO");
		list_add(recursos, recurso);
	}
	configuracion->recursos = recursos;
	for (int i = 0; instancias_recursos_str[i] != NULL; i++)
	{
		free(instancias_recursos_str[i]);
		free(lista_recursos[i]);
	}
	free(instancias_recursos_str);
	free(lista_recursos);

	return configuracion;
}

void manejarGrado()
{
	while (1)
	{
		sem_wait(manejo_grado);
		int valor;
		sem_getvalue(GRADO_MULTIPROGRAMACION, &valor);
		// log_info(logger,"Old semaphore value: %d\n", valor);
		int aux = abs(valor_grado_a_modificar - configuracion->grado_multiprogramacion);
		if (valor_grado_a_modificar > configuracion->grado_multiprogramacion)
		{
			for (int i = 0; i < aux; i++)
			{
				sem_post(GRADO_MULTIPROGRAMACION);
			}
		}
		else if (valor_grado_a_modificar < configuracion->grado_multiprogramacion)
		{

			for (int i = 0; i < aux; i++)
			{
				sem_wait(GRADO_MULTIPROGRAMACION);
			}
		}
	}
}

void evaluar_planificacion(char *planificador)
{
	if (habilitar_planificadores == 0)
	{
		if (strcmp(planificador, "largo") == 0)
		{
			sem_wait(b_reanudar_largo_plazo);
		}
		else if (strcmp(planificador, "corto") == 0)
		{
			sem_wait(b_reanudar_corto_plazo);
		}
		else if (strcmp(planificador, "exit_largo") == 0)
		{
			sem_wait(b_reanudar_exit_largo);
		}
		else if (strcmp(planificador, "exec_blocked") == 0)
		{
			sem_wait(b_reanudar_exec_blocked);
		}
		else if (strcmp(planificador, "exec_ready") == 0)
		{
			sem_wait(b_reanudar_exec_ready);
		}
		else if (strcmp(planificador, "blocked_ready") == 0)
		{
			sem_wait(b_reanudar_blocked_ready);
		}
	}
}

t_pcb *transicion_generica(colaEstado *colaEstadoInicio, colaEstado *colaEstadoFinal, char *planificacion)
{
	t_pcb *proceso = sacar_procesos_cola(colaEstadoInicio, planificacion);
	evaluar_planificacion(planificacion);
	agregar_a_estado(proceso, colaEstadoFinal);

	return proceso;
}

void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado)
{
	pthread_mutex_lock(cola_estado->mutex_estado);
	list_add(cola_estado->estado, pcb);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	sem_post(cola_estado->contador);
}

t_pcb *sacar_procesos_cola_basico(colaEstado *cola_estado, char *planificacion)
{
	sem_wait(cola_estado->contador);
	pthread_mutex_lock(cola_estado->mutex_estado);
	t_pcb *pcb = list_remove(cola_estado->estado, 0);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	return pcb;
}

t_pcb *sacar_procesos_cola(colaEstado *cola_estado, char *planificacion)
{
	evaluar_planificacion(planificacion);
	sem_wait(cola_estado->contador);
	evaluar_planificacion(planificacion);
	pthread_mutex_lock(cola_estado->mutex_estado);
	t_pcb *pcb = list_remove(cola_estado->estado, 0);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	return pcb;
}

colaEstado *obtener_cola(t_estados estado)
{
	switch (estado)
	{
	case NEW:
		return cola_new_global;
		break;
	case READY:
		return cola_ready_global;
		break;
	case EXEC:
		return cola_exec_global;
		break;
	case BLOCKED:
		return cola_bloqueado_global;
		break;
	case FINISHED:
		return cola_exit_global;
		break;
	default:
		log_error(logger, "No se encontro la cola");
		break;
	}
	return NULL;
}

void levantar_CPU_Interrupt()
{
	socket_cpu_interrupt = levantarCliente(logger, "CPU", valores_config->ip_cpu, string_itoa(valores_config->puerto_cpu_interrupt));
}

void levantar_CPU_Dispatch()
{
	socket_cpu_dispatch = levantarCliente(logger, "CPU", valores_config->ip_cpu, string_itoa(valores_config->puerto_cpu_dispatch));

	while (1)
	{
		op_code cod = recibir_op_code(socket_cpu_dispatch);
		if (cod == -1)
		{
			log_error(logger, "CPU se desconectó. Terminando hilo");
			finalizar_hilos();
			pthread_exit((void *)EXIT_FAILURE);
		}
		switch (cod)
		{
		case FINALIZAR_PROCESO:

			tipo_buffer *buffer_cpu_fin = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu_fin);
			motivoFinalizar motivo = leer_buffer_enteroUint32(buffer_cpu_fin);
			destruir_buffer(buffer_cpu_fin);
			finalizar_proceso_success(cde_interrumpido->pid, motivo);
			sem_post(b_largo_plazo_exit);
			break;

		case EXIT_SUCCESS:

			sem_post(sem_finalizar_proceso);
			break;

		case FIN_DE_QUANTUM:

			tipo_buffer *buffer_quantum = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_quantum);
			destruir_buffer(buffer_quantum);
			log_info(logger, "PID: <%d> - Desalojado por fin de Quantum", cde_interrumpido->pid); // log obligatorio
			pthread_cancel(hiloQuantum);
			sem_post(b_transicion_exec_ready);
			break;

		case INSTRUCCION_INTERFAZ:

			tipo_buffer *buffer_interfaz = recibir_buffer(socket_cpu_dispatch); // DATOS DE INTERFAZ

			if (strcmp(valores_config->algoritmo_planificacion, "RR") == 0 || strcmp(valores_config->algoritmo_planificacion, "VRR") == 0)
			{
				pthread_cancel(hiloQuantum);
			}

			tipo_buffer *buffer_interfaz_cde = recibir_buffer(socket_cpu_dispatch); // CDE
			cde_interrumpido = leer_cde(buffer_interfaz_cde);
			sem_post(b_transicion_exec_blocked);
			recibir_orden_interfaces_de_cpu(cde_interrumpido->pid, buffer_interfaz);
			destruir_buffer(buffer_interfaz);
			destruir_buffer(buffer_interfaz_cde);

			break;

		case WAIT_RECURSO:

			tipo_buffer *buffer_wait = recibir_buffer(socket_cpu_dispatch);
			nombre_recurso_recibido = leer_buffer_string(buffer_wait);
			tipo_buffer *buffer_wait_cde = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_wait_cde);
			destruir_buffer(buffer_wait);
			destruir_buffer(buffer_wait_cde);
			if (existe_recurso(nombre_recurso_recibido))
			{
				t_recurso *recurso_SO = obtener_recurso(nombre_recurso_recibido);
				wait_instancia_recurso(recurso_SO);
				free(recurso_SO);
			}
			else
			{
				finalizar_proceso(cde_interrumpido->pid, INVALID_RESOURCE);
				sem_post(b_largo_plazo_exit);
			}
			break;

		case SIGNAL_RECURSO:

			tipo_buffer *buffer_signal = recibir_buffer(socket_cpu_dispatch);
			nombre_recurso_recibido = leer_buffer_string(buffer_signal);
			tipo_buffer *buffer_signal_cde = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_signal_cde);
			destruir_buffer(buffer_signal);
			destruir_buffer(buffer_signal_cde);
			if (existe_recurso(nombre_recurso_recibido))
			{
				t_recurso *recurso = obtener_recurso(nombre_recurso_recibido);
				signal_instancia_recurso(recurso);
				free(recurso);
			}
			else
			{
				finalizar_proceso(cde_interrumpido->pid, INVALID_RESOURCE);
				sem_post(b_largo_plazo_exit);
			}
			break;
		default:
			break;
		}
	}
}

// MANEJO DE INTERFACES DE ENTRADA/SALIDA

_Bool interfaz_esta_en_lista(void *ptr)
{
	t_infoIO *interfaz = (t_infoIO *)ptr;
	return strcmp(interfaz->nombre_io, nombre_IO) == 0;
}

void *conectar_interfaces()
{
	while (1)
	{
		socket_disp_io = esperar_cliente(logger, "KERNEL", "E/S", servidor_para_io);
		op_code operacion = recibir_op_code(socket_disp_io);
		if (operacion == SOLICITUD_CONEXION_IO)
		{
			tipo_buffer *buffer_io = recibir_buffer(socket_disp_io);
			int tipo_interfaz_io = leer_buffer_enteroUint32(buffer_io);
			nombre_IO = leer_buffer_string(buffer_io);
			char *tipo_io = obtener_interfaz(tipo_interfaz_io);

			if (list_find(lista_interfaces, interfaz_esta_en_lista) != NULL)
			{
				enviar_op_code(socket_disp_io, ESTABA_CONECTADO);
			}
			else
			{
				t_infoIO *infoIO = malloc(sizeof(t_infoIO));
				infoIO->cliente_io = socket_disp_io;
				infoIO->nombre_io = nombre_IO;
				infoIO->procesos_espera = list_create();
				infoIO->tipo_IO = tipo_interfaz_io;
				list_add(lista_interfaces, infoIO);

				enviar_op_code(socket_disp_io, NO_ESTABA_CONECTADO);
			}
			destruir_buffer(buffer_io);
		}
	}
	return (void *)-1;
}

bool interfaz_no_esta_conectada(t_infoIO *informacion_interfaz)
{
	op_code codigo = CONFIRMAR_CONEXION;

	// Enviar el código de operación
	int bytes_enviados = send(informacion_interfaz->cliente_io, &codigo, sizeof(op_code), 0);
	if (bytes_enviados == -1)
	{
		return true;
	}

	// Esperar la respuesta del servidor
	op_code respuesta;
	int bytes_recibidos = recv(informacion_interfaz->cliente_io, &respuesta, sizeof(op_code), MSG_WAITALL);
	if (bytes_recibidos <= 0)
	{
		return true;
	}

	// Verificar si la respuesta es OK
	return respuesta != OK;
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
	return NULL;
}

void recibir_orden_interfaces_de_cpu(int pid, tipo_buffer *buffer_con_instruccion)
{
	op_code operacion_desde_cpu_dispatch = leer_buffer_enteroUint32(buffer_con_instruccion);
	t_tipoDeInstruccion instruccion_interfaz = leer_buffer_enteroUint32(buffer_con_instruccion);

	t_infoIO *informacion_interfaz;
	uint32_t tamanioRegistro;
	uint32_t direccion_fisica;
	t_struct_io *io_con_info_buffer = malloc(sizeof(t_struct_io));

	switch (operacion_desde_cpu_dispatch)
	{
	case SOLICITUD_INTERFAZ_GENERICA:

		uint32_t unidades_trabajo = leer_buffer_enteroUint32(buffer_con_instruccion);

		nombre_IO = leer_buffer_string(buffer_con_instruccion);
		io_con_info_buffer->pid = pid;
		io_con_info_buffer->instruccion = instruccion_interfaz;
		io_con_info_buffer->unidades_trabajo = unidades_trabajo;
		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_en_lista);

		list_add(informacion_interfaz->procesos_espera, cde_interrumpido);
		log_info(logger, "PID: <%d> - Bloqueado por : <%s>", pid, nombre_IO);

		free(nombre_IO);

		if (informacion_interfaz == NULL || interfaz_no_esta_conectada(informacion_interfaz))
		{
			interfaz_no_conectada(pid, informacion_interfaz);
		}
		else
		{
			interfaz_conectada(informacion_interfaz, io_con_info_buffer);
		}
		// free(informacion_interfaz);
		break;

	case SOLICITUD_INTERFAZ_STDIN:

		tamanioRegistro = leer_buffer_enteroUint32(buffer_con_instruccion);
		int tamanioMarco = leer_buffer_enteroUint32(buffer_con_instruccion);
		direccion_fisica = leer_buffer_enteroUint32(buffer_con_instruccion);

		nombre_IO = leer_buffer_string(buffer_con_instruccion);
		io_con_info_buffer->tamanio_marco = tamanioMarco;
		io_con_info_buffer->tamanio_reg = tamanioRegistro;
		io_con_info_buffer->dir_fisica = direccion_fisica;
		io_con_info_buffer->pid = pid;
		io_con_info_buffer->instruccion = instruccion_interfaz;
		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_en_lista);

		list_add(informacion_interfaz->procesos_espera, cde_interrumpido);
		log_info(logger, "PID: <%d> - Bloqueado por : <%s>", pid, nombre_IO);

		if (informacion_interfaz == NULL || interfaz_no_esta_conectada(informacion_interfaz))
		{
			interfaz_no_conectada(pid, informacion_interfaz);
		}
		else
		{
			interfaz_conectada(informacion_interfaz, io_con_info_buffer);
		}
		break;

	case SOLICITUD_INTERFAZ_STDOUT:

		tamanioRegistro = leer_buffer_enteroUint32(buffer_con_instruccion);
		direccion_fisica = leer_buffer_enteroUint32(buffer_con_instruccion);

		nombre_IO = leer_buffer_string(buffer_con_instruccion);
		io_con_info_buffer->pid = pid;
		io_con_info_buffer->tamanio_reg = tamanioRegistro;
		io_con_info_buffer->dir_fisica = direccion_fisica;
		io_con_info_buffer->instruccion = instruccion_interfaz;
		informacion_interfaz = list_find(lista_interfaces, interfaz_esta_en_lista);

		list_add(informacion_interfaz->procesos_espera, cde_interrumpido);
		log_info(logger, "PID: <%d> - Bloqueado por : <%s>", pid, nombre_IO);

		if (informacion_interfaz == NULL || interfaz_no_esta_conectada(informacion_interfaz))
		{
			interfaz_no_conectada(pid, informacion_interfaz);
		}
		else
		{
			interfaz_conectada(informacion_interfaz, io_con_info_buffer);
		}
		break;

	case SOLICITUD_INTERFAZ_DIALFS:

		tipo_buffer *buffer_archivo = recibir_buffer(socket_cpu_dispatch);
		char *nombre_archivo = leer_buffer_string(buffer_archivo);
		uint32_t tamanio;
		int marco;
		uint32_t direccion_fisica;
		uint32_t puntero_archivo;
		switch (instruccion_interfaz)
		{
		case IO_FS_CREATE:
		case IO_FS_DELETE:

			nombre_IO = leer_buffer_string(buffer_con_instruccion);
			informacion_interfaz = list_find(lista_interfaces, interfaz_esta_en_lista);

			list_add(informacion_interfaz->procesos_espera, cde_interrumpido);
			log_info(logger, "PID: <%d> - Bloqueado por : <%s>", pid, nombre_IO);

			if (informacion_interfaz == NULL || interfaz_no_esta_conectada(informacion_interfaz))
			{
				interfaz_no_conectada(pid, informacion_interfaz);
			}
			else
			{
				fs_create_delete(nombre_archivo, instruccion_interfaz, informacion_interfaz);
			}
			break;

		case IO_FS_TRUNCATE:
			tamanio = leer_buffer_enteroUint32(buffer_con_instruccion);

			nombre_IO = leer_buffer_string(buffer_con_instruccion);
			informacion_interfaz = list_find(lista_interfaces, interfaz_esta_en_lista);
			list_add(informacion_interfaz->procesos_espera, cde_interrumpido);
			log_info(logger, "PID: <%d> - Bloqueado por : <%s>", pid, nombre_IO);
			if (informacion_interfaz == NULL || interfaz_no_esta_conectada(informacion_interfaz))
			{
				interfaz_no_conectada(pid, informacion_interfaz);
			}
			else
			{
				fs_truncate(nombre_archivo, instruccion_interfaz, tamanio, informacion_interfaz);
			}
			break;

		case IO_FS_WRITE:
		case IO_FS_READ:
			if (instruccion_interfaz == IO_FS_READ)
			{
				marco = leer_buffer_enteroUint32(buffer_con_instruccion);
			}
			tamanio = leer_buffer_enteroUint32(buffer_con_instruccion);
			direccion_fisica = leer_buffer_enteroUint32(buffer_con_instruccion);
			puntero_archivo = leer_buffer_enteroUint32(buffer_con_instruccion);

			nombre_IO = leer_buffer_string(buffer_con_instruccion);
			informacion_interfaz = list_find(lista_interfaces, interfaz_esta_en_lista);
			list_add(informacion_interfaz->procesos_espera, cde_interrumpido);
			log_info(logger, "PID: <%d> - Bloqueado por : <%s>", pid, nombre_IO);
			if (informacion_interfaz == NULL || interfaz_no_esta_conectada(informacion_interfaz))
			{
				interfaz_no_conectada(pid, informacion_interfaz);
			}
			else
			{
				fs_write_read(nombre_archivo, instruccion_interfaz, marco, tamanio, direccion_fisica, puntero_archivo, informacion_interfaz);
			}
			break;
		default:
			break;
		}
		break;
	default:
		log_error(logger, "ERROR - Solicitud Interfaz Enviada Por CPU");
		break;
	}
}

void fs_create_delete(char *nombre_archivo, t_tipoDeInstruccion instruccion_interfaz, t_infoIO *io)
{
	tipo_buffer *buffer_fs = crear_buffer();
	enviar_op_code(io->cliente_io, CONSULTAR_DISPONIBILDAD);
	op_code op_fs = recibir_op_code(io->cliente_io);

	if (op_fs == ESTOY_LIBRE)
	{
		agregar_buffer_para_enterosUint32(buffer_fs, instruccion_interfaz);
		agregar_buffer_para_enterosUint32(buffer_fs, cde_interrumpido->pid);
		agregar_buffer_para_string(buffer_fs, nombre_archivo);

		enviar_buffer(buffer_fs, io->cliente_io);

		op_fs = recibir_op_code(io->cliente_io);

		if (op_fs == CONCLUI_OPERACION)
		{
			sem_post(b_transicion_blocked_ready);
			list_remove(io->procesos_espera, 0);
		}
		else
		{
			log_error(logger, "Hubo un error al crear o eliminar el archivo %s", nombre_archivo);
		}
	}
}

void fs_truncate(char *nombre_archivo, t_tipoDeInstruccion instruccion_interfaz, uint32_t tamanio, t_infoIO *io)
{
	tipo_buffer *buffer_fs = crear_buffer();
	enviar_op_code(io->cliente_io, CONSULTAR_DISPONIBILDAD);
	op_code op_fs = recibir_op_code(io->cliente_io);

	if (op_fs == ESTOY_LIBRE)
	{
		agregar_buffer_para_enterosUint32(buffer_fs, instruccion_interfaz);
		agregar_buffer_para_enterosUint32(buffer_fs, cde_interrumpido->pid);
		agregar_buffer_para_enterosUint32(buffer_fs, tamanio);
		agregar_buffer_para_string(buffer_fs, nombre_archivo);
		enviar_buffer(buffer_fs, io->cliente_io);

		op_fs = recibir_op_code(io->cliente_io);

		if (op_fs == CONCLUI_OPERACION)
		{
			sem_post(b_transicion_blocked_ready);
			list_remove(io->procesos_espera, 0);
		}
	}
}

void fs_write_read(char *nombre_archivo, t_tipoDeInstruccion instruccion_interfaz, uint32_t marco, uint32_t tamanio, uint32_t reg_direccion, uint32_t puntero_archivo, t_infoIO *io)
{
	tipo_buffer *buffer_fs = crear_buffer();
	enviar_op_code(io->cliente_io, CONSULTAR_DISPONIBILDAD);
	op_code op_fs = recibir_op_code(io->cliente_io);

	if (op_fs == ESTOY_LIBRE)
	{
		agregar_buffer_para_enterosUint32(buffer_fs, instruccion_interfaz);
		agregar_buffer_para_enterosUint32(buffer_fs, cde_interrumpido->pid);
		agregar_buffer_para_enterosUint32(buffer_fs, tamanio);
		agregar_buffer_para_enterosUint32(buffer_fs, reg_direccion);
		agregar_buffer_para_enterosUint32(buffer_fs, puntero_archivo);
		if (instruccion_interfaz == IO_FS_READ)
		{
			agregar_buffer_para_enterosUint32(buffer_fs, marco);
		}
		agregar_buffer_para_string(buffer_fs, nombre_archivo);
		enviar_buffer(buffer_fs, io->cliente_io);

		op_fs = recibir_op_code(io->cliente_io);
		if (op_fs == CONCLUI_OPERACION)
		{
			sem_post(b_transicion_blocked_ready);
			list_remove(io->procesos_espera, 0);
		}
	}
}

void interfaz_no_conectada(int pid, t_infoIO *informacion_interfaz)
{
	if (list_remove_element(lista_interfaces, informacion_interfaz))
	{
		finalizar_proceso(pid, INVALID_INTERFACE);
	}
}

void interfaz_conectada(t_infoIO *io, t_struct_io *informacion_buffer)
{
	enviar_op_code(io->cliente_io, CONSULTAR_DISPONIBILDAD);
	op_code operacion_io = recibir_op_code(io->cliente_io);
	tipo_buffer *buffer_interfaz = crear_buffer();

	if (operacion_io == ESTOY_LIBRE)
	{
		enviar_buffer_interfaz(io, informacion_buffer);
		operacion_io = recibir_op_code(io->cliente_io);
		if (operacion_io == CONCLUI_OPERACION)
		{
			sem_post(b_transicion_blocked_ready);
			list_remove(io->procesos_espera, 0);
		}
	}
	else
	{
		log_info(logger, "INTERFAZ: <%s> - OCUPADA POR OTRO PROCESO");
	}
	destruir_buffer(buffer_interfaz);
}

void enviar_buffer_interfaz(t_infoIO *interfaz, t_struct_io *info_buffer)
{
	tipo_buffer *buffer_interfaz = crear_buffer();
	switch (interfaz->tipo_IO)
	{
	case GENERICA:
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->instruccion);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->unidades_trabajo);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->pid);
		break;
	case STDIN:
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->instruccion);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->tamanio_reg);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->tamanio_marco);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->dir_fisica);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->pid);
		break;
	case STDOUT:
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->instruccion);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->tamanio_reg);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->dir_fisica);
		agregar_buffer_para_enterosUint32(buffer_interfaz, info_buffer->pid);
		break;
	default:
		log_error(logger, "Tipo de Interfaz Desconocida");
		break;
	}
	enviar_buffer(buffer_interfaz, interfaz->cliente_io);
	destruir_buffer(buffer_interfaz);
}

// MANEJO DE RECURSOS

bool existe_recurso(char *nombre_recurso)
{
	for (size_t i = 0; i < list_size(valores_config->recursos); i++)
	{
		t_recurso *recurso = list_get(valores_config->recursos, i);

		if (strcmp(recurso->nombre, nombre_recurso) == 0)
		{
			return true;
			break;
		}
	}
	return false;
}

t_recurso *obtener_recurso(char *nombre_recurso)
{
	for (size_t i = 0; i < list_size(valores_config->recursos); i++)
	{
		t_recurso *recurso = list_get(valores_config->recursos, i);
		if (strcmp(recurso->nombre, nombre_recurso) == 0)
		{
			return recurso;
			break;
		}
	}
	return NULL;
}

_Bool ya_tiene_instancias_del_recurso(t_recurso *recurso_proceso)
{
	if (strcmp(nombre_recurso_recibido, recurso_proceso->nombre) == 0)
	{
		return 1;
	}
	return 0;
}

void signal_instancia_recurso(t_recurso *recurso)
{
	int proceso_en_espera;
	t_pcb *proceso;

	sem_getvalue(recurso->cola_bloqueados->contador, &proceso_en_espera);

	if (proceso_en_espera > 0)
	{
		sem_post(b_transicion_blocked_ready);
		proceso = sacar_procesos_cola_basico(recurso->cola_bloqueados, "");
		mostrar_procesos(recurso->cola_bloqueados);
	}

	enviar_a_cpu_cde(cde_interrumpido);
	sem_post(recurso->instancias);
}

void wait_instancia_recurso(t_recurso *recurso)
{
	int valor;
	sem_getvalue(recurso->instancias, &valor);
	t_pcb *proceso_interrumpido = buscar_pcb_en_colas(cde_interrumpido->pid);
	proceso_interrumpido->cde = cde_interrumpido;

	t_recurso *recurso_buscado = list_find(proceso_interrumpido->recursosAsignados, ya_tiene_instancias_del_recurso);

	if (valor > 0)
	{
		sem_wait(recurso->instancias);
		if (recurso_buscado == NULL)
		{
			t_recurso *recurso_asignado = malloc(sizeof(t_recurso));
			recurso_asignado->nombre = nombre_recurso_recibido;
			recurso_asignado->instancias = malloc(sizeof(sem_t));
			sem_init(recurso_asignado->instancias, 0, 1);

			list_add(proceso_interrumpido->recursosAsignados, recurso_asignado);
		}
		else
		{
			sem_post(recurso_buscado->instancias);
		}
		enviar_a_cpu_cde(cde_interrumpido);
	}
	else
	{
		sem_post(b_transicion_exec_blocked);
		agregar_a_estado(proceso_interrumpido, recurso->cola_bloqueados);
	}
}

void asignar_recurso(t_recurso *recurso, t_pcb *pcb)
{
	nombre_recurso_recibido = recurso->nombre;
	t_recurso *recurso_proceso = list_find(pcb->recursosAsignados, ya_tiene_instancias_del_recurso);
	sem_wait(recurso->instancias);

	if (recurso_proceso == NULL)
	{
		t_recurso *recurso_asignado = malloc(sizeof(t_recurso));
		recurso_asignado->nombre = nombre_recurso_recibido;
		recurso_asignado->instancias = malloc(sizeof(sem_t));
		sem_init(recurso_asignado->instancias, 0, 1);
		list_add(pcb->recursosAsignados, recurso_asignado);
	}
	else
	{
		// log_info(logger, "Recurso: <%s> Estaba cargado en el proceso - PID: <%d>", nombre_recurso_recibido, pcb->cde->pid);
		sem_post(recurso_proceso->instancias); // Cant de instancias que retiene el proceso
											   // log_info(logger, "Recurso: <%s> Cargado - PID: <%d>", nombre_recurso_recibido, pcb->cde->pid);
	}
	sem_post(b_transicion_blocked_ready);
}

/* void imprimir_recursos()
{
	for (size_t i = 0; i < list_size(valores_config->recursos); i++)
	{
		t_recurso *recurso = list_get(valores_config->recursos, i);
		int valor;
		sem_getvalue(recurso->instancias, &valor);
		log_info(logger, "Recurso: %s - Cant: %d", recurso->nombre, valor);
	}
} */