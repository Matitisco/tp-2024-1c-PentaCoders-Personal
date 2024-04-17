#include "../include/kernel.h"

struct config_kernel {
	t_config* config;
    char* puerto_escucha;
	char* ip_memoria;
	char* puerto_memoria;
	char* ip_cpu;
	char*puerto_cpu_dispatch;
	char *puerto_cpu_interrupt;	
};

struct config_kernel* config_kernel(){
	struct config_kernel* configuracion = (struct config_kernel*) malloc(sizeof(struct config_kernel));

	configuracion->config = iniciar_config("../kernel.config");

	configuracion->ip_memoria = config_get_string_value(configuracion->config,"IP_MEMORIA");
	configuracion->puerto_memoria = config_get_string_value(configuracion->config,"PUERTO_MEMORIA");
	configuracion->puerto_escucha = config_get_string_value(configuracion->config,"PUERTO_ESCUCHA");
	configuracion->puerto_cpu_dispatch = config_get_string_value(configuracion->config,"PUERTO_CPU_DISPATCH");
	configuracion->puerto_cpu_interrupt= config_get_string_value(configuracion->config,"PUERTO_CPU_INTERRUPT");

	return configuracion;
}

int main(int argc, char* argv[]) {
	int conexion_memoria;
	int conexion_cpu;
	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);

	// CONFIG

	struct config_kernel* configuracion_kernel = config_kernel();

	//KERNEL SE CONECTA A MEMORIA
	conexion_memoria = crear_conexion(logger, "Memoria", configuracion_kernel->ip_memoria, configuracion_kernel->puerto_memoria);
	enviar_mensaje("Kernel  esta conectado a memoria", conexion_memoria);

	//KERNEL SE CONECTA A CPU
	conexion_cpu= crear_conexion(logger, "CPU", configuracion_kernel->ip_cpu, configuracion_kernel->puerto_cpu_dispatch);
	enviar_mensaje("Kernel esta conectado a CPU", conexion_cpu);


	//	KERNEL COMO SERVER
	int server_fd = iniciar_servidor(logger,"Kernel",configuracion_kernel->ip_cpu, configuracion_kernel->puerto_escucha);
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(logger, "Kernel" ,server_fd);

	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cliente_fd);
				break;
			case PAQUETE:
				lista = recibir_paquete(cliente_fd);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				return EXIT_FAILURE;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
	return EXIT_SUCCESS;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

// Va a ser cliente y servidor ya que envia procesos a la cpu y pide cosas a memoria y recibe las interrupciones de los disp de I/O, respectivamente.
