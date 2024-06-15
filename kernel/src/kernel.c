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
char *nombre_recurso_recibido;
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

			if (list_find(lista_interfaces, interfaz_esta_conectada) != NULL)
			{
				log_info(logger, "La Interfaz %s ya esta conectada", nombre_IO);
				enviar_cod_enum(disp_io, ESTABA_CONECTADO);
			}
			else
			{
				enviar_cod_enum(disp_io, NO_ESTABA_CONECTADO);
				log_info(logger, "La Interfaz %s no estaba conectada", nombre_IO);

				t_infoIO *infoIO = malloc(sizeof(t_infoIO));
				infoIO->cliente_io = disp_io;
				infoIO->nombre_io = nombre_IO;
				infoIO->procesos_espera = list_create(); // lista de procesos

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
	// sem_init(b_detener_planificacion_largo, 0, 0);//agregado de lo que hizo Mati
	// sem_init(b_detener_planificacion_corto, 0, 0);//agregado de lo que hizo Mati
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

	//t_recurso **recursos;
	//t_lista_recursos *recursos;




	char **lista_recursos = config_get_array_value(configuracion->config, "RECURSOS"); //{"RA", "RB", "RC", NULL }
	char **instancias_recursos_str = config_get_array_value(configuracion->config, "INSTANCIAS_RECURSOS");// {"1", "2", "1", NULL }


	int tamanio = 0;
	while (instancias_recursos_str[tamanio] != NULL)
	{
		tamanio++;
	}

	t_lista_recursos *recursos = list_create();// lista con elementos de tipo t_recurso

	for (int i = 0; i < tamanio; i++) {// voy creando tantos recursos según hayan en el config->  [RA,RB,RC] -> {"RA", "RB", "RC", NULL }
		t_recurso* recurso = malloc(sizeof(t_recurso));
		recurso->nombre = strdup(lista_recursos[i]);
		//recurso.cola_bloqueados =  //colaEstado *cola_bloqueados;
		recurso->instancias = malloc(sizeof(sem_t));

		int instancias = atoi(instancias_recursos_str[i]); // cant de instancias del recurso
		sem_init(recurso->instancias, 0, instancias);
		list_add(recursos,recurso);
	}
	configuracion->recursos = recursos;
	// Libero los arrays
	for (int i = 0; instancias_recursos_str[i] != NULL; i++)
	{
		free(instancias_recursos_str[i]);
		free(lista_recursos[i]);
	}
	free(instancias_recursos_str);
	free(lista_recursos);


	for (int a = 0; a < tamanio; a++) {
		
		t_recurso *recurso = list_get(configuracion->recursos,a);
		printf("\033[0;33m\n Nombre del recurso: %s \n \033[0m",recurso->nombre);
		int valor_instancias;
		sem_getvalue(recurso->instancias, &valor_instancias);
		printf("\033[0;33m\n Instancias: %d \n \033[0m",valor_instancias);
	}

	return configuracion;
}

void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado)
{
	pthread_mutex_lock(cola_estado->mutex_estado);
	queue_push(cola_estado->estado, pcb);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	sem_post(cola_estado->contador);
}

t_pcb *sacar_procesos_cola(colaEstado *cola_estado)
{
	t_pcb *pcb = malloc(sizeof(t_pcb));
	sem_wait(cola_estado->contador);
	pthread_mutex_lock(cola_estado->mutex_estado);
	pcb = queue_pop(cola_estado->estado);
	pthread_mutex_unlock(cola_estado->mutex_estado);
	return pcb;
}

colaEstado* obtener_cola(t_estados estado){
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
		break;
	}
log_error(logger, "No se encontro la cola");
return NULL;
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
			// IO_GEN_SLEEP INTERFAZ TIEMPO
			// DIN DE QUANTUM Y OCURRE LA INTERRUPCION
			sem_post(b_transicion_exec_ready);

			sem_post(b_reanudar_largo_plazo); // está en el código de Mati
			sem_post(b_reanudar_corto_plazo); // está en el código de Mati

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
			
			printf("\033[0;33m\n WAIT_RECURSO \n \033[0m");

			tipo_buffer *buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu);

			nombre_recurso_recibido = leer_buffer_string(buffer_cpu);
			destruir_buffer(buffer_cpu);

			sem_post(b_transicion_exec_blocked);
			
			if (existe_recurso2(nombre_recurso_recibido)) // encontro al recurso y existe
			{	printf("\033[0;33m\n Existe el recurso: %s \n \033[0m",nombre_recurso_recibido);

				t_recurso * recurso = obtener_recurso(nombre_recurso_recibido);
				wait_instancia_recurso2(recurso); // el SO pierde uno del recurso llamado
				int valor_instancias;

				sem_getvalue(recurso->instancias, &valor_instancias);
				log_info(logger, "Recurso: %s Instancias Restantes: %d", recurso->nombre, valor_instancias);
				
				proceso_interrumpido = buscarProceso(cde_interrumpido->pid);
				t_recurso *recurso_del_proceso = list_find(proceso_interrumpido->recursosAsignados, ya_tiene_instancias_del_recurso); 
				//se supone que si llega acá o bien el recurso ya estaba cargado o se cargó por primera vez -> t_recurso *recurso_del_proceso no debería ser NULL
				
				int recursos_en_espera;

				sem_getvalue(recurso_del_proceso->cola_bloqueados->contador, &recursos_en_espera);
				log_info(logger, "recursos esperando %d", recursos_en_espera);
				if (recursos_en_espera == 0)//si el proceso NO se bloqueo x falta de recurso
					{
						sem_post(b_transicion_blocked_ready);
						sem_post(b_reanudar_largo_plazo);
						sem_post(b_reanudar_corto_plazo);
					} // si se bloqueo, queda esperando
			}
			
		
			else{	
				printf("\033[38;2;255;105;180m \n No existe el recurso: %s \n \033[0m",nombre_recurso_recibido);				

				finalizar_proceso(cde_interrumpido->pid, INVALID_RESOURCE);
				sem_post(b_largo_plazo_exit);
				sem_post(b_reanudar_largo_plazo);
				sem_post(b_reanudar_corto_plazo);
			}
		
			break;
		case SIGNAL_RECURSO: // un proceso libera un recurso
			/*
			buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu);
			nombre_recurso_recibido = leer_buffer_string(buffer_cpu);
			destruir_buffer(buffer_cpu);
			
			sem_post(b_transicion_exec_blocked);

			*/
			//posicion;
			//recurso = existe_recurso(&posicion);
			/*
			if (recurso == 0) // encontro al recurso y existe
			{
				signal_instancia_recurso(posicion);
				// t_pcb *proceso = buscarProceso(cde_interrumpido->pid);
				// t_recurso *recurso_buscado = list_find(proceso->recursosAsignados, ya_tiene_instancias_del_recurso);

				// sem_wait(recurso_buscado->instancias);

				int valor_instancias;

				sem_getvalue(&(valores_config->recursos[posicion]->instancias), &valor_instancias);
				log_info(logger, "Recurso: %s Instancias Restantes: %d", valores_config->recursos[posicion]->nombre, valor_instancias);
				sem_post(b_transicion_blocked_ready);
				sem_post(b_reanudar_largo_plazo);
				sem_post(b_reanudar_corto_plazo);

			}
			else
			{
				log_info(logger, "El Recurso Pedido No Existe En El Sistema");

				finalizar_proceso(cde_interrumpido->pid, INVALID_RESOURCE);
				sem_post(b_largo_plazo_exit);
				sem_post(b_reanudar_largo_plazo);
				sem_post(b_reanudar_corto_plazo);
			}
			*/
			break;
		case OUT_OF_MEMORY:

			buffer_cpu = recibir_buffer(socket_cpu_dispatch);
			cde_interrumpido = leer_cde(buffer_cpu);
			finalizar_proceso(cde_interrumpido->pid, OUT_OF_MEMORY_END);

			sem_post(b_largo_plazo_exit);
			sem_post(b_reanudar_largo_plazo);
			sem_post(b_reanudar_corto_plazo);

			break;
		default:
			log_warning(logger, "La operacion enviada por CPU Dispatch no la Puedo ejecutar");
			break;
		}
	}
}
/*
_Bool ya_tiene_instancias_del_recurso(t_recurso *recurso_proceso)
{
	if (strcmp(recurso_recibido, recurso_proceso->nombre) == 0)
	{
		return 1;
	}
	return 0;
}
*/

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
			interfaz_conectada_generica(unidades_trabajo, instruccion_a_ejecutar, informacion_interfaz, pid);
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
		}
		break;

	case SOLICITUD_INTERFAZ_STDOUT:

		instruccion_a_ejecutar = leer_buffer_enteroUint32(buffer_con_instruccion); // IO_STDOUT_WRITE
		log_info(logger, "INSTURCCION A EJECUTAR STDOUT: %d", instruccion_a_ejecutar);
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

void interfaz_conectada_generica(int unidades_trabajo, t_tipoDeInstruccion instruccion_a_ejecutar, t_infoIO *io, int pid)
{
	enviar_cod_enum(io->cliente_io, CONSULTAR_DISPONIBILDAD);

	op_code operacion_io = recibir_operacion(io->cliente_io);
	tipo_buffer *buffer_interfaz = crear_buffer();

	if (operacion_io == ESTOY_LIBRE)
	{
		agregar_buffer_para_enterosUint32(buffer_interfaz, instruccion_a_ejecutar);
		agregar_buffer_para_enterosUint32(buffer_interfaz, unidades_trabajo);
		agregar_buffer_para_enterosUint32(buffer_interfaz, pid);
		enviar_buffer(buffer_interfaz, io->cliente_io);
		operacion_io = recibir_operacion(io->cliente_io);
		if (operacion_io == CONCLUI_OPERACION)
		{
			sem_post(b_transicion_blocked_ready);
			// fijarnos si hay proceso bloqueados en la lista de interfaz y enviarlos
			/* 			if (list_is_empty(io->procesos_espera))
						{
							sem_post(b_reanudar_largo_plazo);
							sem_post(b_reanudar_corto_plazo);
						}
						else
						{

						} */
			sem_post(b_reanudar_largo_plazo);
			sem_post(b_reanudar_corto_plazo);
		}
	}
	else
	{
		// mandar a bloquear el proceso a la lista de bloqueados de la interfaz
		log_info(logger, "Interfaz Ocupada");
		list_add(io->procesos_espera, cde_interrumpido);
		log_info(logger, "Se agrego el proceso: %d a la lista de pendientes de la interfaz %s", pid, io->nombre_io);
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


bool existe_recurso2(char *nombre_recurso){

	for (size_t i = 0; i < list_size(valores_config->recursos); i++)
	{
		t_recurso * recurso = list_get(valores_config->recursos,i);

		if(strcmp(recurso->nombre,nombre_recurso) == 0){
			return true;
			break;
		}
		//TODO: hacer free para recurso
	}
	return false;
}

t_recurso * obtener_recurso(char *nombre_recurso){ // dado un nombre de un recuso, devuelve el recurso
	t_recurso * recurso = NULL;
	for (size_t i = 0; i < list_size(valores_config->recursos); i++)
	{
		recurso = list_get(valores_config->recursos,i);

		if(strcmp(recurso->nombre,nombre_recurso) == 0){
			return recurso;
			break;
		}
		//TODO: hacer free para recurso
	}
	
	log_error(logger, "NO se encuentra el recurso");

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


	


void wait_instancia_recurso2(t_recurso * recurso) // si entra acá es porque el recurso existe
{
	int valor;

	sem_getvalue(recurso->instancias, &valor);

	if (valor > 0) // Si hay instancias del recurso, puedo restar y asignar a un proceso
	{
		sem_wait(recurso->instancias); // le doy una instancia al proceso, y le quito una a la lista de recursos que tiene el SO
		// De aca para abajo asigna al proceso los recursos retenidos

		proceso_interrumpido = buscarProceso(cde_interrumpido->pid); //PCB

		t_recurso *recurso_buscado = list_find(proceso_interrumpido->recursosAsignados, ya_tiene_instancias_del_recurso); // si no lo encuentra devuelve NULL
		if (recurso_buscado == NULL) // el caso de que el proceso no contaba con el recurso ya cargado
		{
			log_info(logger, "Recurso: <%s> No estaba cargado en proceso - PID: <%d>", nombre_recurso_recibido, proceso_interrumpido->cde->pid);
			t_recurso *recurso_asignado = malloc(sizeof(t_recurso));
			recurso_asignado->nombre = nombre_recurso_recibido;
			recurso_asignado->instancias = malloc(sizeof(sem_t));
			recurso_asignado->cola_bloqueados = constructorColaEstado("BLOCK");
			sem_init(recurso_asignado->instancias, 0, 1);

			list_add(proceso_interrumpido->recursosAsignados, recurso_asignado);
			log_info(logger, "Recurso: <%s> Cargado - PID: <%d>", recurso_asignado->nombre, proceso_interrumpido->cde->pid);
		}
		else // ya tiene el recurso cargado el proceso
		{
			log_info(logger, "Recurso: <%s> Estaba cargado en el proceso - PID: <%d>", nombre_recurso_recibido, proceso_interrumpido->cde->pid);
			sem_post(recurso_buscado->instancias);
			log_info(logger, "Recurso: <%s> Cargado - PID: <%d>", nombre_recurso_recibido, proceso_interrumpido->cde->pid);
		}
	}// recurso_del_proceso->cola_bloqueados->contador
	else
	{
		log_info(logger, "No Hay Instancias Disponibles del Recurso");
		// mandar proceso a cola de bloqueados correspondiente al recurso
		sem_post(b_transicion_exec_blocked);
	}
}
