#include "../include/utils.h"
sem_t *sem_kernel_io_generica;

/*----------------------- GENERAL ------------------*/

// INICIAR LOGGER
t_log *iniciar_logger(char *path_log, char *nombre_log)
{
	t_log *nuevo_logger;
	nuevo_logger = log_create(path_log, nombre_log, 1, LOG_LEVEL_INFO);
	if (nuevo_logger == NULL)
	{
		return NULL;
	};
	return nuevo_logger;
}
// INICIAR CONFIG
t_config *iniciar_config(char *config_path)
{
	t_config *nuevo_config;
	nuevo_config = config_create(config_path);
	if (nuevo_config == NULL)
	{
		printf("\nNo se pudo leer la config\n");
		exit(2);
	}
	return nuevo_config;
}
// TERMINAR PROGRAMA
void terminar_programa(int conexion, t_log *logger, t_config *config)
{
	destruirLog(logger);
	destruirConfig(config);
	liberar_conexion(conexion);
}
// DESTRUIR LOG
void destruirLog(t_log *logger)
{
	if (logger != NULL)
	{
		log_destroy(logger);
	}
}
// DESTRUIR CONFIG
void destruirConfig(t_config *config)
{
	if (config != NULL)
	{
		config_destroy(config);
	}
}
// LIBERAR CONEXION
void liberarConexion(int conexion)
{
	if (conexion != 0)
	{
		liberar_conexion(conexion);
	}
}

// CREAR ARGUMENTOS

t_args *crearArgumento(char *puerto, char *ip)
{
	t_args *a;
	a = malloc(sizeof(t_args));
	a->logger = logger;
	a->puerto = malloc(sizeof(char *));
	a->ip = malloc(sizeof(char *));
	strcpy(a->puerto, puerto);

	strcpy(a->ip, ip);

	return a;
}