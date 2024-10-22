#include "../include/cpu.h"

// VARIABLES GLOBALES
config_cpu *valores_config_cpu;
int interrupcion_rr;

int interrrupcion_fifo;
int interrupcion_entrada_salida;
int interrupcion_io;
int interrupcion_fs;
int socket_memoria;
int socket_kernel_dispatch;
int socket_kernel_interrupt;
int salida_exit;
int desalojo_wait;
int desalojo_signal;
int tamanio_pagina;
char *nombre_archivo_a_enviar;
pthread_t hilo_CPU_CLIENTE;
pthread_t hilo_CPU_SERVIDOR_DISPATCH;
pthread_t hilo_CPU_SERVIDOR_INTERRUPT;
tipo_buffer *buffer_instruccion_io;
pthread_mutex_t *mutex_cde_ejecutando;
sem_t *sem_check_interrupt;
sem_t *sem_interface;
t_cde *cde_recibido;
int interrupcion_exit;
char *ip_local;
t_registros *registros;
pthread_mutex_t mutex_salida_exit;
sem_t *s_exit;

int main(int argc, char *argv[])
{
	s_exit = malloc(sizeof(sem_t));
	sem_init(s_exit, 0, 0);
	interrupcion_exit = 0;
	ip_local = obtener_ip_local();
	printf("IP LOCAL: %s\n", ip_local);
	salida_exit = 1;
	sem_post(s_exit);
	iniciar_modulo_cpu();

	pthread_join(hilo_CPU_SERVIDOR_INTERRUPT, NULL);
	pthread_join(hilo_CPU_SERVIDOR_DISPATCH, NULL);
	pthread_join(hilo_CPU_CLIENTE, NULL);

	config_destroy(valores_config_cpu->config);
	free(valores_config_cpu);

	eliminar_tlb();
	free(registros);
	liberar_conexion(&socket_memoria);
	free(ip_local);
	log_destroy(logger);
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
	sem_interface = (sem_t *)malloc(sizeof(sem_t));
	sem_init(mutex_cde_ejecutando, 0, 0);
	sem_init(sem_check_interrupt, 0, 1);
	sem_init(sem_interface, 0, 0);
	pthread_mutex_init(&mutex_salida_exit, NULL);
}

void crear_hilos_CPU()
{
	pthread_create(&hilo_CPU_CLIENTE, NULL, levantar_conexion_a_memoria, NULL);
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
	int server_fd = crear_servidor(valores_config_cpu->puerto_escucha_dispatch);
	socket_kernel_dispatch = esperar_cliente(logger, "CPU DISPATCH", "Kernel", server_fd);
	while (1)
	{
		op_code codigo = 0;
		codigo = recibir_op_code(socket_kernel_dispatch);

		if (codigo == -1)
		{
			log_error(logger, "El KERNEL se desconecto de dispatch. Terminando servidor");
			pthread_cancel(hilo_CPU_SERVIDOR_INTERRUPT);
			pthread_cancel(hilo_CPU_CLIENTE);
			pthread_exit((void *)EXIT_FAILURE);
		}

		switch (codigo)
		{
		case EJECUTAR_PROCESO:

			tipo_buffer *buffer_cde = recibir_buffer(socket_kernel_dispatch);
			cde_recibido = leer_cde(buffer_cde);
			registros = cde_recibido->registros;
			pthread_mutex_lock(&mutex_salida_exit);
			salida_exit = 1;
			sem_post(s_exit);
			pthread_mutex_unlock(&mutex_salida_exit);
			while (salida_exit)
			{
				//sleep_ms(1000);
				char *linea_instruccion = fetch(cde_recibido);
				log_info(logger, "PID: <%d> - FETCH - Program Counter: <%d>", cde_recibido->pid, cde_recibido->PC);
				cde_recibido->PC++;
				char **array_instruccion = decode(linea_instruccion);
				execute(array_instruccion, cde_recibido);
				check_interrupt();
				string_array_destroy(array_instruccion);
			}
			free(cde_recibido->registros);
			free(cde_recibido);
			destruir_buffer(buffer_cde);
			break;
		default:
			log_error(logger, "Dispatch - Operacion Desconocida - Finalizando Servidor");
			return (void *)EXIT_FAILURE;
			break;
		}
	}
}

void liberar_array_instruccion(char **array_instruccion)
{
	int i = 0;
	while (array_instruccion[i] != NULL)
	{
		free(array_instruccion[i]);
		i++;
	}
	free(array_instruccion);
}

void *levantar_kernel_interrupt()
{
	int server_fd = crear_servidor(valores_config_cpu->puerto_escucha_interrupt);
	int socket_kernel_interrupt = esperar_cliente(logger, "CPU INTERRUPT", "Kernel", server_fd);
	while (1)
	{
		if (salida_exit == 0)
		{
			sem_wait(s_exit);
		}
		op_code codigo = recibir_op_code(socket_kernel_interrupt);
		if (codigo == -1)
		{

			log_error(logger, "El KERNEL se desconecto de interrupt. Terminando servidor");
			printf("\033[0m");
			pthread_cancel(hilo_CPU_SERVIDOR_DISPATCH);
			pthread_cancel(hilo_CPU_CLIENTE);
			pthread_exit((void *)EXIT_FAILURE);
		}

		switch (codigo)
		{
		case PROCESO_INTERRUMPIDO_QUANTUM:
			interrupcion_rr = 1;
			break;
		case SOLICITUD_EXIT:
			pthread_mutex_lock(&mutex_salida_exit);
			interrupcion_exit = 1;
			pthread_mutex_unlock(&mutex_salida_exit);
			break;
		default:
			log_error(logger, "Codigo de operacion desconocido.");
			break;
		}
	}
}

void *levantar_conexion_a_memoria()
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
	enviar_op_code(socket_memoria, PEDIDO_INSTRUCCION);
	tipo_buffer *buffer = crear_buffer();

	agregar_buffer_para_enterosUint32(buffer, contexto->pid);

	agregar_buffer_para_enterosUint32(buffer, contexto->PC);

	contexto->path = NULL;
	enviar_buffer(buffer, socket_memoria);
	destruir_buffer(buffer);

	op_code operacion_desde_memoria = recibir_op_code(socket_memoria);

	if (operacion_desde_memoria == ENVIAR_INSTRUCCION_CORRECTO)
	{
		tipo_buffer *bufferProximaInstruccion = recibir_buffer(socket_memoria);
		char *linea_de_instruccion = leer_buffer_string(bufferProximaInstruccion);
		destruir_buffer(bufferProximaInstruccion);
		return linea_de_instruccion;
	}
	else
	{
		log_error(logger, "CPU - ENVIO INSTRUCCION INCORRECTO");
		return NULL;
	}
}

char **decode(char *linea_de_instrucion)
{

	char **instruccion = string_split(linea_de_instrucion, " ");
	free(linea_de_instrucion);
	return instruccion;
}

void execute(char **instruccion, t_cde *contextoProceso)
{
	t_tipoDeInstruccion cod_instruccion = obtener_instruccion(instruccion[0]);
	switch (cod_instruccion)
	{
	case SET:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_set(instruccion[1], atoi((instruccion[2])));
		actualizar_cde();
		break;
	case MOV_IN:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_mov_in(instruccion[1], instruccion[2], contextoProceso);
		actualizar_cde();
		break;
	case MOV_OUT:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_mov_out(instruccion[1], instruccion[2], contextoProceso);
		actualizar_cde();
		break;
	case SUM:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_sum(instruccion[1], instruccion[2]);
		actualizar_cde();
		break;
	case SUB:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_sub(instruccion[1], instruccion[2]);
		actualizar_cde();
		break;
	case JNZ:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_jnz(instruccion[1], atoi((instruccion[2])), contextoProceso);
		actualizar_cde();
		break;
	case RESIZE:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>", contextoProceso->pid, instruccion[0], instruccion[1]);
		exec_resize(instruccion[1], contextoProceso);
		actualizar_cde();
		break;
	case COPY_STRING:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>", contextoProceso->pid, instruccion[0], instruccion[1]);
		exec_copy_string(instruccion[1], contextoProceso);
		actualizar_cde();
		break;
	case WAIT:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>", contextoProceso->pid, instruccion[0], instruccion[1]);
		exec_wait(instruccion[1], contextoProceso);
		actualizar_cde();
		break;
	case SIGNAL:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>", contextoProceso->pid, instruccion[0], instruccion[1]);
		exec_signal(instruccion[1], contextoProceso);
		actualizar_cde();
		break;
	case IO_GEN_SLEEP:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_io_gen_sleep(instruccion[1], atoi((instruccion[2])));
		actualizar_cde();
		break;
	case IO_STDIN_READ:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2], instruccion[3]);
		exec_io_stdin_read(instruccion[1], instruccion[2], instruccion[3], contextoProceso);
		actualizar_cde();
		break;
	case IO_STDOUT_WRITE:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2], instruccion[3]);
		exec_io_stdout_write(instruccion[1], instruccion[2], instruccion[3], contextoProceso);
		actualizar_cde();
		break;
	case IO_FS_CREATE:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_io_fs_create(instruccion[1], instruccion[2], contextoProceso);
		actualizar_cde();
		break;
	case IO_FS_DELETE:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		exec_io_fs_delete(instruccion[1], instruccion[2], contextoProceso);
		actualizar_cde();
		break;
	case IO_FS_TRUNCATE:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2], instruccion[3]);
		exec_io_fs_truncate(instruccion[1], instruccion[2], instruccion[3], contextoProceso);
		actualizar_cde();
		break;
	case IO_FS_WRITE:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s %s %s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2], instruccion[3], instruccion[4], instruccion[5]);
		exec_io_fs_write(instruccion[1], instruccion[2], instruccion[3], instruccion[4], instruccion[5], contextoProceso);
		actualizar_cde();
		break;
	case IO_FS_READ:
		log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s %s %s %s %s>", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2], instruccion[3], instruccion[4], instruccion[5]);
		exec_io_fs_read(instruccion[1], instruccion[2], instruccion[3], instruccion[4], instruccion[5], contextoProceso);
		actualizar_cde();
		break;
	case EXIT:
		interrupcion_rr = 0;
		log_info(logger, "PID: <%d> - Ejecutando <%s> ", contextoProceso->pid, instruccion[0]);
		actualizar_cde();
		exec_exit(contextoProceso, SUCCESS);
		break;
	default:
		log_error(logger, "No encontre la instruccion");
		break;
	}
}

void check_interrupt()
{
	tipo_buffer *buffer_cde = crear_buffer();
	if (interrupcion_rr)
	{
		pthread_mutex_lock(&mutex_salida_exit);
		salida_exit = 0;
		pthread_mutex_unlock(&mutex_salida_exit);
		interrupcion_rr = 0;
		if (interrupcion_io)
		{
			interrupcion_io = 0;
			agregar_cde_buffer(buffer_cde, cde_recibido);
			enviar_buffer(buffer_cde, socket_kernel_dispatch); // enviamos proceso interrumpido
		}
		else if (interrupcion_fs)
		{
			interrupcion_fs = 0;
			agregar_cde_buffer(buffer_cde, cde_recibido);
			enviar_buffer(buffer_cde, socket_kernel_dispatch);
			tipo_buffer *buffer_archivo = crear_buffer();
			agregar_buffer_para_string(buffer_archivo, nombre_archivo_a_enviar);
			enviar_buffer(buffer_archivo, socket_kernel_dispatch);
			destruir_buffer(buffer_archivo);
		}
		else if (interrupcion_exit)
		{
			pthread_mutex_lock(&mutex_salida_exit);
			interrupcion_exit = 0;
			pthread_mutex_unlock(&mutex_salida_exit);
			exec_exit(cde_recibido, INTERRUPTED_BY_USER);
		}
		else
		{
			enviar_op_code(socket_kernel_dispatch, FIN_DE_QUANTUM);

			agregar_cde_buffer(buffer_cde, cde_recibido);
			enviar_buffer(buffer_cde, socket_kernel_dispatch);
		}
	}
	else if (interrupcion_exit)
	{
		pthread_mutex_lock(&mutex_salida_exit);
		interrupcion_exit = 0;
		pthread_mutex_unlock(&mutex_salida_exit);
		exec_exit(cde_recibido, INTERRUPTED_BY_USER);
	}
	else if (interrupcion_io) // sin quantum
	{
		pthread_mutex_lock(&mutex_salida_exit);
		salida_exit = 0;
		pthread_mutex_unlock(&mutex_salida_exit);
		interrupcion_io = 0;
		agregar_cde_buffer(buffer_cde, cde_recibido);
		enviar_buffer(buffer_cde, socket_kernel_dispatch);
	}
	else if (interrupcion_fs)
	{
		pthread_mutex_lock(&mutex_salida_exit);
		salida_exit = 0;
		pthread_mutex_unlock(&mutex_salida_exit);
		interrupcion_fs = 0;
		agregar_cde_buffer(buffer_cde, cde_recibido);
		enviar_buffer(buffer_cde, socket_kernel_dispatch);
		tipo_buffer *buffer_archivo = crear_buffer();
		agregar_buffer_para_string(buffer_archivo, nombre_archivo_a_enviar);
		enviar_buffer(buffer_archivo, socket_kernel_dispatch);
		destruir_buffer(buffer_archivo);
	}
	else if (desalojo_signal)
	{
		pthread_mutex_lock(&mutex_salida_exit);
		salida_exit = 0;
		pthread_mutex_unlock(&mutex_salida_exit);
		desalojo_signal = 0;

		agregar_cde_buffer(buffer_cde, cde_recibido);
		enviar_buffer(buffer_cde, socket_kernel_dispatch);
	}
	else if (desalojo_wait)
	{
		pthread_mutex_lock(&mutex_salida_exit);
		salida_exit = 0;
		pthread_mutex_unlock(&mutex_salida_exit);
		desalojo_wait = 0;
		agregar_cde_buffer(buffer_cde, cde_recibido);
		enviar_buffer(buffer_cde, socket_kernel_dispatch);
	}

	destruir_buffer(buffer_cde);
	return;
}

void actualizar_cde()
{
	cde_recibido->registros = registros;
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
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	char *ultimo_dir = basename(directorioActual);
	if (strcmp(ultimo_dir, "bin") == 0)
	{
		chdir("..");
		getcwd(directorioActual, sizeof(directorioActual));
	}
	strcat(directorioActual, "/cpu.config");
	valores_config_cpu->config = iniciar_config(directorioActual);
	valores_config_cpu->ip = config_get_string_value(valores_config_cpu->config, "IP_MEMORIA");
	valores_config_cpu->puerto_memoria = config_get_string_value(valores_config_cpu->config, "PUERTO_MEMORIA");
	valores_config_cpu->puerto_escucha_dispatch = config_get_int_value(valores_config_cpu->config, "PUERTO_ESCUCHA_DISPATCH");
	valores_config_cpu->puerto_escucha_interrupt = config_get_int_value(valores_config_cpu->config, "PUERTO_ESCUCHA_INTERRUPT");
	valores_config_cpu->algoritmo_tlb = config_get_string_value(valores_config_cpu->config, "ALGORITMO_TLB");
	valores_config_cpu->cantidad_entradas_tlb = config_get_int_value(valores_config_cpu->config, "CANTIDAD_ENTRADAS_TLB");

	return valores_config_cpu;
}