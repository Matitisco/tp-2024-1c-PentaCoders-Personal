#include <../include/cpu.h>


struct config_cpu{
	t_config* config;
	char *ip_memoria;
	char *puerto_memoria;
	//char* puerto_escucha_dispatch;
	//char* puerto_escucha_interrupt;
	//char* algoritmo_tlb;
	//int cantidad_entradas_tlb;
};



struct config_cpu* config_cpu(){
	struct config_cpu* valores_config = malloc(sizeof(struct config_cpu));

	//creo el config
	valores_config->config = iniciar_config("../cpu.config");


	valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP_MEMORIA");
	valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_MEMORIA");


	return valores_config;
}


int main(int argc, char *argv[])
{
	
	int conexion_memoria;
	t_log *logger;

	/* ---------------- LOGGING ---------------- */

	logger = log_create("Cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
	logger = iniciar_logger("cpu.log", "CPU");


	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	struct config_cpu* valores_config = config_cpu();	


	
	// CPU SE CONECTA A MEMORIA 				Falta que CPU inicie servidor como INTERRUPT y como DISPATCH

	conexion_memoria = crear_conexion(logger, "Memoria", valores_config->ip_memoria, valores_config->puerto_memoria);

	enviar_mensaje("CPU Se conecto a Memoria", conexion_memoria);

	// CPU COMO SERVER DE KERNEL
	int server_fd = iniciar_servidor(logger, "SERVIDOR CPU", valores_config->ip_memoria, valores_config->puerto_memoria);
	log_info(logger, "Servidor CPU listo para recibir al cliente KERNEL");
	int cliente_fd = esperar_cliente(logger, "Kernel", server_fd);

	t_list *lista;
	while (1)
	{
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op)
		{
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void *)iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}

void iterator(char *value)
{
	log_info(logger, "%s", value);
}

/*CPU es cliente de memoria y servidor de Kernel*/
