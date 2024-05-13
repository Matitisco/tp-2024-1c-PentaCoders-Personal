#include "../include/cpu.h"

config_cpu *valores_config_cpu;

int CONEXION_A_MEMORIA;
int socket_memoria;

pthread_t hilo_CPU_CLIENTE;
pthread_t hilo_CPU_SERVIDOR_DISPATCH;
pthread_t hilo_CPU_SERVIDOR_INTERRUPT;
// Semaforos
// mutex_cde_ejecutando;
t_args *args_memoria;
t_args *kernel_ds;
t_args *kernel_int;
pthread_mutex_t *mutex_cde_ejecutando;

int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");
	valores_config_cpu = configurar_cpu(); // CONFIG

	iniciar_registros();

	iniciar_hilos_CPU(valores_config_cpu);
	iniciar_semaforos_CPU();
	pthread_join(hilo_CPU_CLIENTE, NULL);
	pthread_join(hilo_CPU_SERVIDOR_INTERRUPT, NULL);
	pthread_join(hilo_CPU_SERVIDOR_DISPATCH, NULL);

	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config_cpu->config);
}
void iniciar_hilos_CPU(config_cpu *valores_config_cpu)
{
	args_memoria = crearArgumento(valores_config_cpu->puerto_memoria, valores_config_cpu->ip);
	kernel_ds = crearArgumento(valores_config_cpu->puerto_escucha_dispatch, valores_config_cpu->ip);
	kernel_int = crearArgumento(valores_config_cpu->puerto_escucha_interrupt, valores_config_cpu->ip);
	crearHilos_CPU(args_memoria, kernel_ds, kernel_int);
}
void iniciar_semaforos_CPU()
{
	mutex_cde_ejecutando = malloc(sizeof(pthread_mutex_t));
	sem_init(mutex_cde_ejecutando, 0, 0);
}
void crearHilos_CPU(t_args *args_memoria, t_args *kernel_int, t_args *kernel_dis)
{
	pthread_create(&hilo_CPU_CLIENTE, NULL, conexionAMemoria, (void *)args_memoria);
	pthread_create(&hilo_CPU_SERVIDOR_DISPATCH, NULL, levantar_Kernel_Dispatch, (void *)kernel_dis);
	pthread_create(&hilo_CPU_SERVIDOR_INTERRUPT, NULL, levantar_Kernel_Interrupt, (void *)kernel_int);
}
void iniciar_registros()
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

void levantar_Kernel_Dispatch(void *ptr)
{
	t_args *argumento = malloc(sizeof(t_args));
	argumento = (t_args *)ptr;
	int server_fd = iniciar_servidor(argumento->logger,"CPU Dispatch",argumento->ip,argumento->puerto);
	log_info(logger, "Esperando KERNEL DISPATCH....");
	int socket_kernel_dispatch = esperar_cliente(logger, "SERVIDOR CPU DISPATCH", argumento->puerto);
	log_info(logger, "Se conecto el Kernel por DISPATCH");
	while (1)
	{
		op_code codigo = recibir_operacion(socket_kernel_dispatch);
		tipo_buffer *buffer = recibir_buffer(socket_kernel_dispatch);

		switch (codigo)
		{
		case EJECUTAR_PROCESO:

			t_cde *cde_recibido = leer_cde(buffer); // Deserealiza y Arma el CDE
			destruir_buffer(buffer);

			pthread_mutex_lock(&mutex_cde_ejecutando);
			// pid_ejecutar = cde_recibido->pid;
			pthread_mutex_unlock(&mutex_cde_ejecutando);

			// solicitar_instruccion(cde_recibido);
			break;

		case CDE:

			break;

		default:
			// destruir_buffer_nuestro(buffer);
			log_error(logger, "Codigo de operacion desconocido.");
			log_error(logger, "Finalizando modulo.");
			exit(1);
			break;
		}
	}
}

void levantar_Kernel_Interrupt(void *ptr)
{
	t_args *argumento = malloc(sizeof(t_args));
	argumento = (t_args *)ptr;
	log_info(logger, "Esperando KERNEL INTERRUPT....");
	int socket_kernel_interrupt = esperar_cliente(logger, "SERVIDOR CPU INTERRUPT", argumento->puerto);
	log_info(logger, "Se conecto el Kernel por Interrupt");
	while (1)
	{
		op_code codigo = recibir_operacion(socket_kernel_interrupt);
		tipo_buffer *buffer = recibir_buffer(socket_kernel_interrupt);
		switch (codigo)
		{
		case INTERRUPT:
			break;
		default:
			// destruir_buffer_nuestro(buffer);
			log_error(logger, "Codigo de operacion desconocido.");
			log_error(logger, "Finalizando modulo.");
			exit(1);
			break;
		}
	}
}
void *conexionAMemoria(void *ptr)
{
	t_args *argumento = malloc(sizeof(t_args));
	argumento = (t_args *)ptr;
	socket_memoria = levantarCliente(logger, "MEMORIA", argumento->ip, argumento->puerto, "CPU SE CONECTO A MEMORIA");
	free(argumento);
}
char *fetch()
{
	t_cde *cde = malloc(sizeof(t_cde));
	enviar_cod_enum(socket_memoria, PEDIDO_INSTRUCCION); // Pido la instruccion
	tipo_buffer *buffer = crear_buffer();
	// actualizo el buffer escribiendo
	agregar_buffer_para_enterosUint32(buffer, cde->pid);		  // consigo el procoeso asoc
	agregar_buffer_para_enterosUint32(buffer, cde->registro->PC); // con esto la memoria busca la prox ins a ejecutar
	enviar_buffer(buffer, socket_memoria);

	destruir_buffer(buffer);
	tipo_buffer *bufferProximaInstruccion = recibir_buffer(socket_memoria);	   // memoria devuelvo SET AX 1
	char *linea_de_instruccion = leer_buffer_string(bufferProximaInstruccion); // obtenemos la linea instruccion

	cde->registro->PC++; // actualizo el contexto de ejercicion
	return linea_de_instruccion;
}
char **decode(char *linea_de_instrucion)
{
	char **instruccion = string_split(linea_de_instrucion, " ");
	return instruccion; //["SET","AX","1"]
}
// Contexto de ejecucion
void execute(char **instruccion, t_cde *contextoProceso) // recibimos un array
{
	t_tipoDeInstruccion cod_instruccion = obtener_instruccion(instruccion[0]);
	switch (cod_instruccion)
	{
	case SET: // SET AX 1
		exec_set(instruccion[1], atoi((instruccion[2])));
		actualizar_cde(contextoProceso, instruccion);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
		break;
	case MOV_IN:
		actualizar_cde(contextoProceso, instruccion);
	case MOV_OUT:
		actualizar_cde(contextoProceso, instruccion);
	case SUM: // SUM AX BX
		exec_sum(instruccion[1], instruccion[2]);
		actualizar_cde(contextoProceso, instruccion);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
	case SUB: // SUB AX BX
		exec_sub(instruccion[1], instruccion[2]);
		actualizar_cde(contextoProceso, instruccion);
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
	case JNZ: // JNZ AX 4
		actualizar_cde(contextoProceso, instruccion);
		exec_jnz(instruccion[1], atoi((instruccion[2])));
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
	case RESIZE:
		actualizar_cde(contextoProceso, instruccion);
	case COPY_STRING:
		actualizar_cde(contextoProceso, instruccion);
	case WAIT:
		actualizar_cde(contextoProceso, instruccion);
	case SIGNAL:
		actualizar_cde(contextoProceso, instruccion);
	case IO_GEN_SLEEP:
		actualizar_cde(contextoProceso, instruccion);
		exec_io_gen_sleep(instruccion[1], atoi((instruccion[2])));
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando: %s - %s %s", contextoProceso->pid, instruccion[0], instruccion[1], instruccion[2]);
	case IO_STDIN_READ:
		actualizar_cde(contextoProceso, instruccion);
	case IO_STDOUT_WRITE:
		actualizar_cde(contextoProceso, instruccion);
	case IO_FS_CREATE:
		actualizar_cde(contextoProceso, instruccion);
	case IO_FS_DELETE:
		actualizar_cde(contextoProceso, instruccion);
	case IO_FS_TRUNCATE:
		actualizar_cde(contextoProceso, instruccion);
	case IO_FS_WRITE:
		actualizar_cde(contextoProceso, instruccion);
	case IO_FS_READ:
		actualizar_cde(contextoProceso, instruccion);
	case EXIT:
		actualizar_cde(contextoProceso, instruccion);
		exec_exit();
		log_info(logger, "Instrucción Ejecutada: PID: %d - Ejecutando %s ", contextoProceso->pid, instruccion[0]);
	default:
		log_info(logger, "No encontre la instruccion");
		break;
	}
}
void actualizar_cde(t_cde *contexto, char **instruccion)
{
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