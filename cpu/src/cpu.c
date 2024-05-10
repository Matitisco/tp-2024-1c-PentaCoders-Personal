#include "../include/cpu.h"

config_cpu *valores_config_cpu;

int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");
	valores_config_cpu = configurar_cpu(); // CONFIG
	diccionario_instrucciones = dictionary_create();
	iniciar_dic_instrucciones();
	iniciar_registros();

	// CPU es servidor de Kernel (conexion INTERRUPT y DISPATCH) y cliente de Memoria

	// CPU como server de KERNEL
	// levantarServidor(logger, valores_config->puerto_escucha_dispatch, valores_config->ip, "SERVIDOR CPU DISPATCH");
	// levantarServidor(logger, valores_config->puerto_escucha_interrupt,valores_config->ip, "SERVIDOR CPU INTERRUPT");

	servidorDeKernel(valores_config_cpu); // Usado para recibir un PCB

	levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config_cpu->ip, valores_config_cpu->puerto_memoria, "CPU SE CONECTO A MEMORIA");

	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config_cpu->config);
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
void servidorDeKernel(config_cpu *valores_config_cpu)
{
	int socket_servidor_dispatch = iniciar_servidor(logger, "SERVIDOR CPU DISPATCH", valores_config_cpu->ip, valores_config_cpu->puerto_escucha_dispatch);
	log_info(logger, "Servidor: %s listo para recibir al cliente", "SERVIDOR CPU DISPATCH");
	proceso_dispatch(socket_servidor_dispatch); // recibir PCB

	// int cliente_fd = esperar_cliente(logger, "CLIENTE CONECTADO DISPATCH", socket_servidor_dispatch);//espero a que se conecte kernel y recibe el socket

	// int socket_servidor_interrupt = iniciar_servidor(logger,"SERVIDOR CPU INTERRUPT",valores_config->ip,valores_config->puerto_escucha_interrupt);
}

void proceso_dispatch(int socket_servidor_dispatch)
{

	log_info(logger, "Esperando KERNEL DISPATCH....");
	int socket_kernel_dispatch = esperar_cliente(logger, "SERVIDOR CPU DISPATCH", socket_servidor_dispatch);
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
			pid_ejecutar = cde_recibido->pid;
			pthread_mutex_unlock(&mutex_cde_ejecutando);

		
			solicitar_instruccion(cde_recibido);
			break;
		case INTERRUPT:

			break;
		case DESALOJO:

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

/*

	EJECUTAR_PROCESO,
	INTERRUPT,
	DESALOJO,
	CDE
*/
/*
	CONEXION_A_MEMORIA = levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config->ip, valores_config->puerto_memoria, "CPU SE CONECTO A MEMORIA");
	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config->config);
*/

// Borrar? Usamos dispatchProceso
/*
void solicitar_instruccion()
{
	// Pedimos a memoria instruccion

	// armamos buffer
	// send
	// despes recv

	// segun la instruccion que recibimos ejecutamos ciclo de instruccion

	// ejecutar_ciclo(instruccion);
}
*/
void ejecutar_ciclo()
{
}
/*
void recibir_cde()
{
	mensaje_kernel_cpu codigo = recibir_operacion(socket_kernel_dispatch);
	tipo_buffer *buffer = recibir_buffer(socket_kernel_dispatch);
	if (codigo == EJECUTAR_PROCESO)
	{
		ejecutar_proceso();
	}
}
*/

void solicitar_instruccion(t_cde *cde)
{
	enviar_codigo(socket_memoria, PEDIDO_INSTRUCCION); // Pido la instruccion
	tipo_buffer *buffer = crear_buffer();
	agregar_buffer_para_enterosUint32(buffer, cde->pid);
	agregar_buffer_para_enterosUint32(buffer, cde->pc);
	enviar_buffer(buffer, socket_memoria);
	cde->pc++; // actualizo el contexto de ejercicion
	destruir_buffer(buffer);
	
	//t_instrucion *instruccion_a_ejecutar = leer_buffer(otro_buffer); Falta implementar al funcion leer_buffer
	//destruir_buffer(buffer);
	// ejecutar_proceso();
}
/*
void recibir_instruccion_memoria()
{
	tipo_buffer *uffer= recibir_buffer(socket_memoria);
	 =leer_buffer_enteroUint32(buffer);
	=leer_buffer_enteroUint32(buffer);


}*/

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

	cde->registro->PC; // actualizo el contexto de ejercicion
	return linea_de_instruccion;
}
char *decode(char *linea_de_instrucion)
{
	char **instruccion = string_split(linea_de_instrucion, " ");
	char *operacion = instruccion[0];
	return operacion; //["SET","AX","1"]
}
// Contexto de ejecucion
void execute(char **instruccion)
{
	dictionary_get(diccionario_instrucciones, instruccion);
	// "SET"
	// dictionary_put(diccionario_instrucciones, "SET", exec_set);
	// Buscar en el diccionar esa instruccion
	// devolver esa instruccion con el execute
	// listo
}

void ejecutar_proceso(t_cde*cde_recibido){


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