#include <../include/memoria.h>


struct config_memoria {
	t_config* config;
    char* ip_memoria;
	char* puerto_memoria;
	//int tam_memoria;
	//int tam_pagina;
	//char* path_instrucciones;
	//int retardo_respuesta;
};

struct config_memoria* config_memoria(){
	struct config_memoria* valores_config = malloc(sizeof(struct config_memoria));

	//creo el config
	valores_config->config = iniciar_config("../memoria.config");

	valores_config->ip_memoria = config_get_string_value(valores_config->config,"IP");
	valores_config->puerto_memoria = config_get_string_value(valores_config->config,"PUERTO_ESCUCHA");

	return valores_config;
}


int main(int argc, char* argv[]) {
    
	logger = log_create("server.log", "Memoria", 1, LOG_LEVEL_DEBUG);
	
	struct config_memoria* valores_config = config_memoria();	//config



	
	int server_fd = iniciar_servidor(logger, "Memoria", valores_config->ip_memoria, valores_config->puerto_memoria);


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

	free(valores_config);
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

// MEMORIA ES SERVIDOR DE TODOS
