#include "../include/entrada_salida.h"

int main(int argc, char *argv[])
{
	/* ---------------- LOGGING ---------------- */

	logger = iniciar_logger("entrada_salida.log", "ENTRADA_SALIDA");
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config = iniciar_config("entrada_salida.config");

	ip = config_get_string_value(config, "IP_KERNEL");
	PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
	PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");

	/* ---------------- LEER DE CONSOLA ---------------- */

	leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/
	levantarCliente(conexion_kernel, logger, "Servidor Kernel", ip, PUERTO_KERNEL, "ENTRADA/SALIDA se conecta a Kernel");

	levantarCliente(conexion_memoria, logger, "Servidor Memoria", ip, PUERTO_MEMORIA, "ENTRADA/SAL|IDA se conecta a Memoria");

	// terminar_programa(conexion_memoria, logger, config);
	// terminar_programa(conexion_kernel, logger, config);
}

// FUNCIONES

void leer_consola(t_log *logger)
{
	char *leido;
	leido = readline("> ");
	log_info(logger, "%s", leido);
	while (strcmp(leido, "") != 0)
	{
		free(leido);
		leido = readline("> ");
		log_info(logger, "%s", leido);
	}
	free(leido);
}
