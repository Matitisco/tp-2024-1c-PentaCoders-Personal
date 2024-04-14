#include "../include/kernel.h"


int main(int argc, char* argv[]) {
	char* puerto_escucha;

	char* ip;
	char* puerto_memoria;
	int conexion_memoria;
	int conexion_cpu;
	char*puerto_cpu;
	//char*puerto_cpu_dispatch; //esto va si va el tema de los puertos esos
	
	//char *puerto_cpu_interrupt;
	int conexion_cpu;
	t_config* config;
	
	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);


	// CONFIG
	config = iniciar_config("kernel.config");

	ip = config_get_string_value(config,"IP_MEMORIA");
	puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");


	//KERNEL SE CONECTA A MEMORIA
	conexion_memoria = crear_conexion(logger, "Memoria", ip, puerto_memoria);
	enviar_mensaje("hola", conexion_memoria);

	 //KERNEL SE CONECTA A CPU
	
	
    puerto_cpu = config_get_string_value(config,"PUERTO_CPU"); //Este va si no va la parte del dispatch y el interrupt
	conexion_cpu= crear_conexion(logger, "CPU", ip, puerto_cpu);
	enviar_mensaje("aloo", conexion_cpu);
	/*
	 puerto_cpu_dispatch= config_get_string_value(config,"PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt= config_get_istring_value(config,"PUERTO_CPU_INTERRUPT");
	 */




	//	KERNEL COMO SERVER
	int server_fd = iniciar_servidor(logger,"Kernel",ip, puerto_escucha);
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
