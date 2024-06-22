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


int string_to_int(char *str) {
    int result = 0;
    int i = 0;
    int sign = 1;

    // Manejo del signo
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    // Convertir cada dígito a un número entero
    for (; str[i] != '\0'; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        } else {
            // Si el carácter no es un dígito, salir o manejar el error según sea necesario
            printf("Error: No es un número válido\n");
            exit(EXIT_FAILURE);
        }
    }

    return result * sign;
}

void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}