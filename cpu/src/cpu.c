#include "../include/cpu.h"

// VARIABLES GLOBALES
config_cpu *valores_config_cpu;
int interrupcion_rr;
int interrrupcion_fifo;
int interrupcion_entrada_salida;
int interrupcion_io;
int CONEXION_A_MEMORIA;
int socket_memoria;
int socket_kernel_dispatch;
int socket_kernel_interrupt;
int salida_exit;
int desalojo_wait;
int desalojo_signal;
int tamanio_pagina;
pthread_t hilo_CPU_CLIENTE;
pthread_t hilo_CPU_SERVIDOR_DISPATCH;
pthread_t hilo_CPU_SERVIDOR_INTERRUPT;
tipo_buffer *buffer_instruccion_io;
pthread_mutex_t *mutex_cde_ejecutando;
sem_t *sem_check_interrupt;
t_cde *cde_recibido;

int main(int argc, char *argv[])
{
	iniciar_modulo_cpu();

	pthread_join(hilo_CPU_SERVIDOR_INTERRUPT, NULL);
	pthread_join(hilo_CPU_SERVIDOR_DISPATCH, NULL);
	pthread_join(hilo_CPU_CLIENTE, NULL);

	// terminar_programa(CONEXION_A_MEMORIA, logger, valores_config_cpu->config);
}

void iniciar_modulo_cpu()
{
	logger = iniciar_logger("cpu.log", "CPU");
	valores_config_cpu = configurar_cpu();

	tlb_iniciar(valores_config_cpu->algoritmo_tlb, valores_config_cpu->cantidad_entradas_tlb);

	iniciar_registros_sistema();

	interrupcion_rr = 0;
	interrrupcion_fifo = 0;
	interrupcion_entrada_salida = 0;

	crear_hilos_CPU();
	iniciar_semaforos_CPU();
}

void iniciar_semaforos_CPU()
{
	mutex_cde_ejecutando = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	sem_check_interrupt = (sem_t *)malloc(sizeof(sem_t));
	sem_init(mutex_cde_ejecutando, 0, 0);
	sem_init(sem_check_interrupt, 0, 1);
}

void crear_hilos_CPU()
{
	pthread_create(&hilo_CPU_CLIENTE, NULL, (void *(*)(void *))levantar_conexion_a_memoria, NULL);
	pthread_create(&hilo_CPU_SERVIDOR_DISPATCH, NULL, levantar_kernel_dispatch, NULL);
	pthread_create(&hilo_CPU_SERVIDOR_INTERRUPT, NULL, levantar_kernel_interrupt, NULL);
}

void iniciar_registros_sistema()
{
	registros = malloc(sizeof(t_registros));
	registros->AX = 0;
	registros->BX = 0;
	registros->CX = 0;
	registros->DX = 0;
	registros->EAX = 0;
	registros->EBX = 0;
	registros->ECX = 0;
	registros->EDX = 0;
	registros->DI = 0;
	registros->SI = 0;
}

void *levantar_kernel_dispatch()
{
	int server_fd = iniciar_servidor(logger, "CPU Dispatch", valores_config_cpu->ip, valores_config_cpu->puerto_escucha_dispatch);
	socket_kernel_dispatch = esperar_cliente(logger, "CPU DISPATCH", "Kernel", server_fd);
	while (1)
	{
		op_code codigo = recibir_operacion(socket_kernel_dispatch);

		if (codigo == -1)
		{
			log_error(logger, "El KERNEL se desconecto de dispatch. Terminando servidor");
			return (void *)EXIT_FAILURE;
		}

		switch (codigo)
		{
		case EJECUTAR_PROCESO:

			tipo_buffer *buffer_cde = recibir_buffer(socket_kernel_dispatch);
			cde_recibido = leer_cde(buffer_cde);
			salida_exit = 1;

			while (salida_exit)
			{
				char *linea_instruccion = fetch(cde_recibido);
				log_info(logger, "PID: <%d> - FETCH - Program Counter: <%d>", cde_recibido->pid, cde_recibido->PC);
				cde_recibido->PC++;
				char **array_instruccion = decode(linea_instruccion);
				execute(array_instruccion, cde_recibido);
				check_interrupt();
			}

			destruir_buffer(buffer_cde);
			break;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

void *levantar_kernel_interrupt()
{
	int server_fd = iniciar_servidor(logger, "CPU Interrupt", valores_config_cpu->ip, valores_config_cpu->puerto_escucha_interrupt);
	int socket_kernel_interrupt = esperar_cliente(logger, "CPU INTERRUPT", "Kernel", server_fd);
	while (1)
	{
		if (salida_exit)
		{
			op_code codigo = recibir_operacion(socket_kernel_interrupt);
			log_info(logger, "Me llego una interrupcion");

			if (codigo == -1)
			{
				log_error(logger, "El KERNEL se desconecto de interrupt. Terminando servidor");
				return (void *)EXIT_FAILURE;
			}

			switch (codigo)
			{
			case PROCESO_INTERRUMPIDO_QUANTUM:
				interrupcion_rr = 1;
				break;
			case SOLICITUD_EXIT:
				tipo_buffer *buffer_kernel = recibir_buffer(socket_kernel_interrupt); // recibo el buffer de kernel
				int pid = leer_buffer_enteroUint32(buffer_kernel);					  // id del proceso
				destruir_buffer(buffer_kernel);
				// si esta en cpu entonces mandamos a cpu_interrupt una interrupcion
				// pidiendo que desaloje el proceso de la cpu y retorne el cde

				// guardamos en una lista las interrupciones que luego va a ser leida por check_interrupt aplicar semaforos mutex
				break;
			default:
				log_error(logger, "Codigo de operacion desconocido.");
				log_error(logger, "Finalizando modulo.");
				exit(1);
				break;
			}
		}
	}
}

void levantar_conexion_a_memoria()
{
	socket_memoria = levantarCliente(logger, "MEMORIA", valores_config_cpu->ip, valores_config_cpu->puerto_memoria);
	recibir_tamanio_pagina(socket_memoria);
}

void recibir_tamanio_pagina(int socket_memoria)
{
	tipo_buffer *buffer_tamanio = recibir_buffer(socket_memoria);
	tamanio_pagina = leer_buffer_enteroUint32(buffer_tamanio);
	destruir_buffer(buffer_tamanio);
}

char *fetch(t_cde *contexto)
{
	enviar_cod_enum(socket_memoria, PEDIDO_INSTRUCCION);
	tipo_buffer *buffer = crear_buffer();

	agregar_buffer_para_enterosUint32(buffer, contexto->pid);

	agregar_buffer_para_enterosUint32(buffer, contexto->PC);

	contexto->path = NULL;
	enviar_buffer(buffer, socket_memoria);
	destruir_buffer(buffer);

	op_code operacion_desde_memoria = recibir_operacion(socket_memoria);

	if (operacion_desde_memoria == ENVIAR_INSTRUCCION_CORRECTO)
	{
		tipo_buffer *bufferProximaInstruccion = recibir_buffer(socket_memoria);
		char *linea_de_instruccion = leer_buffer_string(bufferProximaInstruccion);
		destruir_buffer(bufferProximaInstruccion);
		return linea_de_instruccion;
	}
	else
	{
		log_error(logger, "CPU: <ENVIO_INSTRUCCION_INCORRECTO>");
		return NULL;
	}
}

char **decode(char *linea_de_instrucion)
{
	char **instruccion = string_split(linea_de_instrucion, " ");
	return instruccion;
}

void execute(char **instruccion, t_cde *contextoProceso)
{
	t_tipoDeInstruccion cod_instruccion = obtener_instruccion(instruccion[0]);
	switch (cod_instruccion)
	{
	case SET:
		exec_set(instruccion[1], atoi((instruccion[2])));
		actualizar_cde(contextoProceso);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		break;
	case MOV_IN:
		exec_mov_in(instruccion[1], instruccion[2], contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		break;
	case MOV_OUT:
		exec_mov_out(instruccion[1], instruccion[2], contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		break;
	case SUM:
		exec_sum(instruccion[1], instruccion[2]);
		actualizar_cde(contextoProceso);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		break;
	case SUB:
		exec_sub(instruccion[1], instruccion[2]);
		actualizar_cde(contextoProceso);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		break;
	case JNZ:
		exec_jnz(instruccion[1], atoi((instruccion[2])), contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		break;
	case RESIZE:
		exec_resize(instruccion[1], contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "PID: %d - Ejecutando: %s - %s", contextoProceso->pid, instruccion[0], instruccion[1]);
		break;
	case COPY_STRING:
		exec_copy_string(instruccion[1], contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "PID: %d - Ejecutando: %s - %s", contextoProceso->pid, instruccion[0], instruccion[1]);
		break;
	case WAIT:
		exec_wait(instruccion[1], contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "PID: %d - Ejecutando: %s - %s", contextoProceso->pid, instruccion[0], instruccion[1]);
		break;
	case SIGNAL:
		exec_signal(instruccion[1], contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "PID: %d - Ejecutando: %s - %s", contextoProceso->pid, instruccion[0], instruccion[1]);
		break;
	case IO_GEN_SLEEP:
		exec_io_gen_sleep(instruccion[1], atoi((instruccion[2])));
		actualizar_cde(contextoProceso);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		break;
	case IO_STDIN_READ:
		exec_io_stdin_read(instruccion[1], instruccion[2], instruccion[3], contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "PID: %d - Ejecutando: %s - %s %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2], instruccion[3]);
		break;
	case IO_STDOUT_WRITE:
		exec_io_stdout_write(instruccion[1], instruccion[2], instruccion[3], contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "PID: %d - Ejecutando: %s - %s %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2], instruccion[3]);
		break;
	case IO_FS_CREATE:
		actualizar_cde(contextoProceso);
		break;
	case IO_FS_DELETE:
		actualizar_cde(contextoProceso);
		break;
	case IO_FS_TRUNCATE:
		actualizar_cde(contextoProceso);
		break;
	case IO_FS_WRITE:
		actualizar_cde(contextoProceso);
		break;
	case IO_FS_READ:
		actualizar_cde(contextoProceso);
		break;
	case EXIT:
		interrupcion_rr = 0;
		exec_exit(contextoProceso);
		actualizar_cde(contextoProceso);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando %s ", contextoProceso->pid, instruccion[0]);
		break;
	default:
		log_info(logger, "No encontre la instruccion");
		break;
	}
}

void check_interrupt()
{
	tipo_buffer *buffer_cde = crear_buffer();
	if (interrupcion_rr)
	{
		salida_exit = 0;
		interrupcion_rr = 0;
		log_info(logger, "\nINTERRUPCION - FIN DE QUANTUM - \n");
		if (interrupcion_io)
		{
			interrupcion_io = 0;
			agregar_cde_buffer(buffer_cde, cde_recibido);
			enviar_buffer(buffer_cde, socket_kernel_dispatch);			  // enviamos proceso interrumpido
			enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch); // enviamos info de interfaz y su instruccion a ejecutar
		}
		else
		{
			enviar_cod_enum(socket_kernel_dispatch, FIN_DE_QUANTUM);
			agregar_cde_buffer(buffer_cde, cde_recibido);
			enviar_buffer(buffer_cde, socket_kernel_dispatch);
		}
	}
	else if (interrrupcion_fifo)
	{
		salida_exit = 0;
		interrrupcion_fifo = 0;
		// enviar_cod_enum(socket_kernel_dispatch, BLOQUEADO_POR_IO);
		agregar_cde_buffer(buffer_cde, cde_recibido);
		enviar_buffer(buffer_cde, socket_kernel_dispatch);
		log_info(logger, "\nINTERRUPCION - BLOCK - \n");
	}
	else if (interrupcion_io)
	{
		salida_exit = 0;
		interrupcion_io = 0;
		agregar_cde_buffer(buffer_cde, cde_recibido);
		enviar_buffer(buffer_cde, socket_kernel_dispatch);			  // enviamos proceso interrumpido
		enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch); // enviamos info de interfaz y su instruccion a ejecutar
		destruir_buffer(buffer_instruccion_io);
	}
	else if (desalojo_signal)
	{
		salida_exit = 0;
		desalojo_signal = 0;
	}
	else if (desalojo_wait)
	{
		salida_exit = 0;
		desalojo_wait = 0;
	}
	else
	{
		log_info(logger, "No hay interrupciones");
	}
	destruir_buffer(buffer_cde);
}

void actualizar_cde(t_cde *contexto)
{
	contexto->registros = registros;
}

t_tipoDeInstruccion obtener_instruccion(char *instruccion)
{
	if (strcmp("SET", instruccion) == 0)
	{
		return SET;
	}
	if (strcmp("MOV_IN", instruccion) == 0)
	{
		return MOV_IN;
	}
	if (strcmp("MOV_OUT", instruccion) == 0)
	{
		return MOV_OUT;
	}
	if (strcmp("SUM", instruccion) == 0)
	{
		return SUM;
	}
	if (strcmp("SUB", instruccion) == 0)
	{
		return SUB;
	}
	if (strcmp("JNZ", instruccion) == 0)
	{
		return JNZ;
	}
	if (strcmp("RESIZE", instruccion) == 0)
	{
		return RESIZE;
	}
	if (strcmp("COPY_STRING", instruccion) == 0)
	{
		return COPY_STRING;
	}
	if (strcmp("WAIT", instruccion) == 0)
	{
		return WAIT;
	}
	if (strcmp("SIGNAL", instruccion) == 0)
	{
		return SIGNAL;
	}
	if (strcmp("IO_GEN_SLEEP", instruccion) == 0)
	{
		return IO_GEN_SLEEP;
	}
	if (strcmp("IO_STDIN_READ", instruccion) == 0)
	{
		return IO_STDIN_READ;
	}
	if (strcmp("IO_STDOUT_WRITE", instruccion) == 0)
	{
		return IO_STDOUT_WRITE;
	}
	if (strcmp("IO_FS_CREATE", instruccion) == 0)
	{
		return IO_FS_CREATE;
	}
	if (strcmp("IO_FS_DELETE", instruccion) == 0)
	{
		return IO_FS_DELETE;
	}
	if (strcmp("IO_FS_TRUNCATE", instruccion) == 0)
	{
		return IO_FS_TRUNCATE;
	}
	if (strcmp("IO_FS_WRITE", instruccion) == 0)
	{
		return IO_FS_WRITE;
	}
	if (strcmp("IO_FS_READ", instruccion) == 0)
	{
		return IO_FS_READ;
	}
	if (strcmp("EXIT", instruccion) == 0)
	{
		return EXIT;
	}
	return -1;
}

config_cpu *configurar_cpu()
{
	config_cpu *valores_config_cpu = malloc(sizeof(config_cpu));

	valores_config_cpu->config = iniciar_config("cpu.config");
	valores_config_cpu->ip = config_get_string_value(valores_config_cpu->config, "IP_MEMORIA");
	valores_config_cpu->puerto_memoria = config_get_string_value(valores_config_cpu->config, "PUERTO_MEMORIA");
	valores_config_cpu->puerto_escucha_dispatch = config_get_string_value(valores_config_cpu->config, "PUERTO_ESCUCHA_DISPATCH");
	valores_config_cpu->puerto_escucha_interrupt = config_get_string_value(valores_config_cpu->config, "PUERTO_ESCUCHA_INTERRUPT");
	valores_config_cpu->algoritmo_tlb = config_get_string_value(valores_config_cpu->config, "ALGORITMO_TLB");
	valores_config_cpu->cantidad_entradas_tlb = config_get_int_value(valores_config_cpu->config, "CANTIDAD_ENTRADAS_TLB");

	return valores_config_cpu;
}