#include <../include/entrada_salida.h>


struct config_io{
	t_config* config;
	char *ip;
	char *puerto_kernel;
	char *puerto_memoria;
	//char* tipo_interfaz;
	//int tiempo_unidad_trabajo;
	//char *ip_kernel;
	//char *ip_memoria;
	//path_base_dialfs;
	//int block_size;
	//int block_count;
};



struct config_io* config_io(){
	struct config_io* valores_config = malloc(sizeof(struct config_io));

	//creo el config
	valores_config->config = iniciar_config("../entrada_salida.config");

	valores_config->ip = config_get_string_value(valores_config->config, "IP_KERNEL");
	valores_config->puerto_kernel = config_get_string_value(valores_config->config, "PUERTO_KERNEL");
	valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_MEMORIA");


	return valores_config;
}




int main(int argc, char *argv[]){

	int conexion;
	int conexion_memoria;

	/* ---------------- LOGGING ---------------- */

	t_log *logger = iniciar_logger("entrada_salida.log","ENTRADA_SALIDA");
	log_info(logger, "Hola! Soy un log");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */


	struct config_io* valores_config = config_io();	//config
	
	
	


	/* ---------------- LEER DE CONSOLA ---------------- */

	leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	// I/O a Kernel
	conexion = crear_conexion(logger, "Kernel", valores_config->ip, valores_config->puerto_kernel);
	enviar_mensaje("entradaSalida conectado a Kernel", conexion);
	paquete(conexion);

	// I/O a Memoria
	conexion_memoria = crear_conexion(logger, "Memoria", valores_config->ip, valores_config->puerto_memoria);
	enviar_mensaje("entradaSalida conectado a Memoria", conexion_memoria);
	//paquete(conexion_memoria);

	terminar_programa(conexion_memoria, logger, valores_config->config);
	terminar_programa(conexion, logger, valores_config->config);
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
