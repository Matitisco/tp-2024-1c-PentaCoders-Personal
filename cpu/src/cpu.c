#include "../include/cpu.h"

config_cpu *valores_config_cpu;

int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");
	valores_config_cpu = configurar_cpu(); // CONFIG

	// CPU es servidor de Kernel (conexion INTERRUPT y DISPATCH) y cliente de Memoria

	// CPU como server de KERNEL
	// levantarServidor(logger, valores_config->puerto_escucha_dispatch, valores_config->ip, "SERVIDOR CPU DISPATCH");
	// levantarServidor(logger, valores_config->puerto_escucha_interrupt,valores_config->ip, "SERVIDOR CPU INTERRUPT");

	servidorDeKernel(valores_config_cpu); // Usado para recibir un PCB

	levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config_cpu->ip, valores_config_cpu->puerto_memoria, "CPU SE CONECTO A MEMORIA");

	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config_cpu->config);
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

			/* while(1){
				//instruccion = solicitar_instruccion(cde_recibido);
				solicitar_instruccion(cde_recibido);
				//ejecutar();
			} */
			// Liberar memoria de cde_recibido al terminar
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

void solicitar_instruccion()
{
	// Pedimos a memoria instruccion

	// armamos buffer
	// send
	// despes recv

	// segun la instruccion que recibimos ejecutamos ciclo de instruccion

	// ejecutar_ciclo(instruccion);
}

void ejecutar_ciclo()
{
}

/*
void pedirInstruccionAMemoria()
{
	t_cde *cde;
	enviar_codigo(socket_memoria, PEDIDO_INSTRUCCION); // Pido la instruccion
	tipo_buffer *buffer = crear_buffer();
	envio_buffer(buffer, socket_memoria);
	destroy_buffer((buffer));
	// actualizo el buffer escribiendo
	escribo_buffer(envio_buffer, cde->pid);
	escribo_buffer(envio_buffer, cde->pc);
	envio_buffer(buffer, socket_memoria);
	cde->pc++; // actualizo el contexto de ejercicion
	destroy_buffer(buffer);
	tipo_buffer *otro_buffer= recibir_buffer_propio(socket_memoria);
	//t_instrucion *instruccion_a_ejecutar = leer_buffer(otro_buffer); Falta implementar al funcion leer_buffer
	//destruir_buffer(buffer);

}*/
/*
typedef struct {
	uint32_t size; // Tamaño del payload
	uint32_t offset; // Desplazamiento dentro del payload
	void* stream; // Payload
} t_buffer;

typedef struct {
	uint8_t codigo_operacion;
	t_buffer* buffer;
} t_paquete;

ssize_t send(int socket, const void *buffer, size_t length <-ESO (Tamaño buffer), int flags);

ssize_t recv(int socket, void *buffer, size_t length <-ESO (Tamaño buffer), int flags);
void *memcpy(void *dest, const void *src, size_t n);


socket es global y ya está definido antes

t_paquete* paquete = malloc(sizeof(t_paquete));
paquete->buffer = malloc(sizeof(t_buffer));

// Primero recibimos el codigo de operacion
recv(unSocket, &(paquete->codigo_operacion), sizeof(uint8_t), 0);

// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
recv(unSocket, &(paquete->buffer->size), sizeof(uint32_t), 0);
paquete->buffer->stream = malloc(paquete->buffer->size);
recv(unSocket, paquete->buffer->stream, paquete->buffer->size, 0);
*/

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