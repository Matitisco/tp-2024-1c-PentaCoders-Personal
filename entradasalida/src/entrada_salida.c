#include <../include/entrada_salida.h>

int main(int argc, char *argv[])
{
	int conexion;
	char *ip;
	char *puerto;
	char *valor;

	t_log *logger;
	t_config *config;

	/* ---------------- LOGGING ---------------- */

	logger = iniciar_logger("entrada_salida.log","ENTRADA_SALIDA");
	log_info(logger, "Hola! Soy un log");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config = iniciar_config("src/entrada_salida.config");

	valor = config_get_string_value(config, "CLAVE");
	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");

	/* ---------------- LEER DE CONSOLA ---------------- */

	leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	conexion = crear_conexion(ip, puerto);
	enviar_mensaje(valor, conexion);
	paquete(conexion);
	terminar_programa(conexion, logger, config);
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

void paquete(int conexion)
{
	char *leido = NULL;
	t_paquete *paquete = crear_paquete();

	leido = readline(">");
	while (strcmp(leido, "") != 0)
	{
		agregar_a_paquete(paquete, leido, strlen(leido) - 1);
		free(leido);
		leido = readline("> ");
	}
	free(leido);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
	if (logger != NULL)
	{
		log_destroy(logger);
	}
	if (config != NULL)
	{
		config_destroy(config);
	}
	if (conexion != 0)
	{
		liberar_conexion(conexion);
	}
}
