#include "../include/cpu.h"

int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");
	struct config_cpu *valores_config = config_cpu();
	diccionario_instrucciones = dictionary_create();
	iniciar_dic_instrucciones();
	iniciar_registros();

	servidorDeKernel(valores_config);

	levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "CPU SE CONECTO A MEMORIA");

	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config->config);
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

/* void servidorDeKernel(struct config_cpu *valores_config)
{
	int socket_servidor_dispatch = iniciar_servidor(logger, "SERVIDOR CPU DISPATCH", valores_config->ip_kernel, valores_config->puerto_escucha_dispatch);
	log_info(logger, "Servidor: %s listo para recibir al cliente", "SERVIDOR CPU DISPATCH");
	int socket_servidor_interrupt = iniciar_servidor(logger, "SERVIDOR CPU INTERRUPT", valores_config->ip_kernel, valores_config->puerto_escucha_interrupt);
	log_info(logger, "Servidor: %s listo para recibir al cliente", "SERVIDOR CPU INTERRUPT");

	int cliente_fd = esperar_cliente(logger, "SERVIDOR CPU DISPATCH", socket_servidor_dispatch); // espero a que se conecte kernel

	proceso_dispatch(socket_servidor_dispatch);
} */

void proceso_dispatch(void *socket_server)
{

	int socket_servidor_dispatch = (int)(intptr_t)socket_server;

	log_info(logger, "Esperando Kernel DISPATCH....");
	socket_kernel_dispatch = esperar_cliente(logger, "SERVIDOR CPU DISPATCH", socket_servidor_dispatch); // En la librería faltaba utils/conexiones.h, ya no hace falta agreagarlo porque se encuentra en instruccionescpu.h
	log_info(logger, "Se conecto el Kernel por DISPATCH");

	while (1)
	{

		mensaje_kernel_cpu op_code = recibir_operacion(socket_kernel_dispatch); // Se recibe de Kernel la operación

		tipo_buffer *buffer = recibir_buffer(socket_kernel_dispatch); // Se recibe de Kernel el PCB

		switch (op_code)
		{
		case EJECUTAR_PROCESO:

			t_cde *cde_recibido = recibir_buffer(buffer);
			destruir_buffer(buffer);

			pthread_mutex_lock(&mutex_cde_ejecutando);
			pid_ejecutar = cde_recibido->pid;
			pthread_mutex_unlock(&mutex_cde_ejecutando);

			solicitar_instruccion(cde_recibido);

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
	CONEXION_A_MEMORIA = levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "CPU SE CONECTO A MEMORIA");
	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config->config);
*/

struct config_cpu *config_cpu()
{
	struct config_cpu *valores_config = malloc(sizeof(struct config_cpu));

	// creo el config
	valores_config->config = iniciar_config("cpu.config");
	valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP_MEMORIA");
	valores_config->ip_kernel = config_get_string_value(valores_config->config, "IP_KERNEL");
	valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_MEMORIA");
	valores_config->puerto_escucha_dispatch = config_get_string_value(valores_config->config, "PUERTO_ESCUCHA_DISPATCH");

	return valores_config;
}

// Borrar? Usamos dispatchProceso
void recibir_cde()
{
	mensaje_kernel_cpu codigo = recibir_operacion(socket_kernel_dispatch);
	tipo_buffer *buffer = recibir_buffer(socket_kernel_dispatch);
	if (codigo == EJECUTAR_PROCESO)
	{
		ejecutar_proceso();
	}
}

void solicitar_instruccion()
{
	// Pedimos a memoria instruccion
	// ejecutamos proceso
	// ejecutar_proceso();
}

void ejecutar_proceso()
{
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

/*
// Function to execute an instruction
void execute(char **instruccion) {
	// Retrieve the instruction type from the first element of the array
	char *instructionType = instruccion[0];

	// Look up the instruction type in the dictionary
	void (*instructionFunction)(char **);
	instructionFunction = (void (*)(char **))dictionary_get(diccionario_instrucciones, instructionType);

	// Check if the instruction type was found in the dictionary
	if (instructionFunction == NULL) {
		printf("Error: Unknown instruction type: %s\n", instructionType);
		return;
	}

	// Execute the corresponding instruction function
	instructionFunction(instruccion);
}

*/
