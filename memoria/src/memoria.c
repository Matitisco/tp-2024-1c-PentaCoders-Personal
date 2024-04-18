#include "../include/memoria.h"

int main(int argc, char *argv[])
{

	logger = iniciar_logger("memoria.log", "MEMORIA");
	config_memoria = iniciar_config("memoria.config");

	ip_memoria = config_get_string_value(config_memoria, "IP");
	puerto_memoria = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");

	// LEVANTAMOS EL SERVIDOR DE MEMORIA
	levantarServidor(logger, puerto_memoria, ip_memoria, "SERVIDOR MEMORIA");

	// terminar_programa(NULL, logger, config_memoria);
	config_destroy(config_memoria);
	log_destroy(logger);
}
