#include <../include/cpu.h>

int main(int argc, char* argv[]) {
    char* puerto_escucha;

	char* ip;
	
    t_config* config;
	
	logger = log_create("Cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);



	// CONFIG
	config = iniciar_config("cpu.config");
     /* LA CPU COMO CLIENTE DE MEMORIA */


    /* LA CPU COMO SERVER DE KERNEL */

   int server_fd = iniciar_servidor(logger,"SERVIDOR CPU",ip, puerto_escucha);
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
