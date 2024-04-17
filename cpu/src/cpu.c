#include <../include/cpu.h>

int main(int argc, char *argv[])
{
	char *PUERTO_MEMORIA, *IP;
	int CONEXION_MEMORIA;
	t_config *config;
	t_log *logger;

	logger = log_create("Cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
	logger = iniciar_logger("cpu.log", "CPU");
	// CONFIG
	config = iniciar_config("../cpu.config");
	IP = config_get_string_value(config, "IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	CONEXION_MEMORIA = crear_conexion(logger, "Memoria", IP, PUERTO_MEMORIA);
	// CPU SE CONECTA A MEMORIA
	/*
	Falta que CPU inicie servidor como INTERRUPT y como DISPATCH
	Kernel a CPU está a la mitad
	*/

	/* MEMORIA COMO SERVER DE CPU*/

	enviar_mensaje("CPU Se conecto a Memoria", CONEXION_MEMORIA);

	// CPU COMO SERVER DE KERNEL
	int server_fd = iniciar_servidor(logger, "SERVIDOR CPU", IP, PUERTO_MEMORIA);
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
