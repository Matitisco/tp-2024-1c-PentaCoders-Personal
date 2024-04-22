#include "../include/memoria.h"


int main(int argc, char *argv[])
{
	logger = iniciar_logger("memoria.log", "MEMORIA");
	struct config_memoria *valores_config = config_memoria();

	// LEVANTAMOS EL SERVIDOR DE MEMORIA
	levantarServidor(logger, valores_config->puerto_memoria, valores_config->ip_memoria, "SERVIDOR MEMORIA");

	destruirConfig(valores_config->config);
	destruirLog(logger);
}