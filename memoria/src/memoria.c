#include <../include/memoria.h>

int main(int argc, char* argv[]) {
    
	t_config* config_memoria;

	char* ip_memoria;
	char* puerto_memoria;
	


    logger = log_create("server.log", "Memoria", 1, LOG_LEVEL_DEBUG);

	config_memoria = iniciar_config("memoria.config");

	ip_memoria = config_get_string_value(config_memoria,"IP");
	puerto_memoria = config_get_string_value(config_memoria,"PUERTO_ESCUCHA");

	int server_fd = iniciar_servidor(logger, "Memoria", ip_memoria, puerto_memoria);


	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(logger, "Memoria", server_fd);

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
