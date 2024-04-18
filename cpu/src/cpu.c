#include <../include/cpu.h>

int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");

	// CONFIG
	struct config_cpu *valores_config = config_cpu();
	// Falta que CPU inicie servidor como INTERRUPT y como DISPATCH
	levantarServidor(logger, valores_config->puerto_escucha_dispatch, valores_config->ip_kernel, "SERVIDOR CPU");
	levantarCliente(CONEXION_A_MEMORIA, logger, "SERVIDOR  MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "CPU SE CONECTO A MEMORIA");
	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config->config);
}
