#include <../include/cpu.h>

int main(int argc, char* argv[]) {
    char* puerto_memoria;

	char* ip;
	
    t_config* config;
	
	logger = log_create("Cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);



	// CONFIG
	config = iniciar_config("cpu.config");


     /* LA CPU COMO CLIENTE DE MEMORIA */
	//tengo que hacer CPU como cliente de memoria
	/*
	Falta que CPU inicie servidor como INTERRUPT y como DISPATCH
	Kernel a CPU está a la mitad
	*/
	ip = config_get_string_value(config,"IP_MEMORIA");
	puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
	
	int conexion_memoria = crear_conexion(logger,"Memoria",ip, puerto_memoria);

	t_log* logger = iniciar_logger("cpu.log", "CPU");
	enviar_mensaje("Hola soy CPU",conexion_memoria);

/*	
	uint32_t handshake = 1;
	uint32_t result;

	send(conexion, &handshake, sizeof(uint32_t), 0);
	recv(conexion, &result, sizeof(uint32_t), MSG_WAITALL);

	if(result == -1){
		log_error(logger, "No se pudo realizar el handshake \n");
	}else if (result == 0) {
		log_info(logger, "Handshake realizado con exito \n");
	}

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor, conexion);
	log_info(logger, "Mensaje enviado \n");
	paquete(conexion);
	terminar_programa(conexion, logger, config);
*/

    /* LA CPU COMO SERVER DE KERNEL */

   	int server_fd = iniciar_servidor(logger,"SERVIDOR CPU",ip, puerto_memoria);
	log_info(logger, "Servidor KERNEL listo para recibir al cliente CPU");
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


void iterator(char* value){
	log_info(logger,"%s", value);
}



/*CPU es cliente de memoria y servidor de Kernel*/
