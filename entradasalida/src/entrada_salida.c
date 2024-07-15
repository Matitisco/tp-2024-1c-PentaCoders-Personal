#include "../include/entrada_salida.h"

config_io *config_interfaz;

int main(int argc, char *argv[])
{
	iniciar_modulo_io();
	liberar_modulo_io();
}

void iniciar_modulo_io()
{
	printf("--------------------------");
	char *nombre_interfaz = readline("Ingrese el nombre de la interfaz: ");
	printf("--------------------------");
	char *path_configuracion = readline("Ingrese el nombre del archivo con la configuracion de la interfaz (sin '.config'): ");
	printf("--------------------------");
	logger = iniciar_logger("entrada_salida.log", nombre_interfaz);
	strcat(path_configuracion, ".config");
	levantar_interfaz(nombre_interfaz, path_configuracion);
	free(nombre_interfaz);
	free(path_configuracion);
}

void liberar_modulo_io()
{
	log_destroy(logger);
	config_destroy(config_interfaz->config);
	free(config_interfaz);
}

void levantar_interfaz(char *nombre, char *PATH)
{
	inicializar_config_IO(PATH);
	estoy_libre = 1;
	t_interfaz *interfaz = crear_interfaz(config_interfaz, nombre);
	if (interfaz == NULL)
	{
		log_error(logger, "ERROR - CREAR INTERFAZ <%s> ", nombre);
	}
	else
	{
		arrancar_interfaz(interfaz);
	}
}

void arrancar_interfaz(t_interfaz *interfaz)
{

	if (interfaz->tipo_interfaz == GENERICA)
	{
		conectarse_kernel(interfaz);
	}
	else
	{
		conectarse_kernel(interfaz);
		conectarse_memoria(interfaz);
	}
	while (1)
	{
		op_code consulta_kernel = recibir_op_code(conexion_kernel);
		if (consulta_kernel == CONSULTAR_DISPONIBILDAD)
		{
			if (estoy_libre)
			{
				enviar_op_code(conexion_kernel, ESTOY_LIBRE);
				realizar_operacion(interfaz);
				enviar_op_code(conexion_kernel, CONCLUI_OPERACION);
			}
			else
			{
				enviar_op_code(conexion_kernel, NO_ESTOY_LIBRE);
			}
		}
		else if (consulta_kernel == CONFIRMAR_CONEXION)
		{
			enviar_op_code(conexion_kernel, OK);
			log_info(logger, "ESTA KERNEL CONECTADO");
		}
		else
		{
			log_error(logger, "ERROR - CONSULTA KERNEL DESCONOCIDA");
		}
	}
}

void conectarse_kernel(t_interfaz *interfaz)
{
	conexion_kernel = levantarCliente(logger, "KERNEL", interfaz->ip_kernel, string_itoa(interfaz->puerto_kernel));

	if (conexion_kernel == 0)
	{
		log_error(logger, "No pude conectarme al Kernel o esta apagado");
		exit(1);
	}

	enviar_op_code(conexion_kernel, SOLICITUD_CONEXION_IO);
	tipo_buffer *buffer_interfaz_kernel = crear_buffer();
	agregar_buffer_para_enterosUint32(buffer_interfaz_kernel, interfaz->tipo_interfaz);
	agregar_buffer_para_string(buffer_interfaz_kernel, interfaz->nombre_interfaz);
	enviar_buffer(buffer_interfaz_kernel, conexion_kernel);
	destruir_buffer(buffer_interfaz_kernel);

	op_code mensaje_kernel = recibir_op_code(conexion_kernel);
	if (mensaje_kernel == ESTABA_CONECTADO)
	{
		log_info(logger, "YA ESTOY CONECTADA CON KERNEL");
		exit(1);
	}
	else if (mensaje_kernel == NO_ESTABA_CONECTADO)
	{
		log_info(logger, "CONEXION EXITOSA CON KERNEL");
	}
}

void conectarse_memoria(t_interfaz *interfaz)
{
	conexion_memoria = levantarCliente(logger, "MEMORIA", interfaz->ip_memoria, string_itoa(interfaz->puerto_memoria));

	if (conexion_memoria == 0)
	{
		log_error(logger, "No pude conectarme a Memoria o esta apagada");
		exit(1);
	}
}

t_interfaz *crear_interfaz(config_io *config, char *nombre)
{
	t_interfaz *interfaz = malloc(sizeof(t_interfaz));

	interfaz->nombre_interfaz = nombre;
	interfaz->ip_kernel = config->ip_kernel;
	interfaz->puerto_kernel = config->puerto_kernel;
	interfaz->tiempo_unidad_trabajo = config->tiempo_unidad_trabajo;

	switch (asignar_interfaz(config->tipo_interfaz))
	{
	case GENERICA:
		interfaz->tipo_interfaz = GENERICA;
		break;
	case STDIN:
		interfaz->tipo_interfaz = STDIN;
		interfaz->ip_memoria = config->ip_memoria;
		interfaz->puerto_memoria = config->puerto_memoria;
		break;
	case STDOUT:
		interfaz->tipo_interfaz = STDOUT;
		interfaz->ip_memoria = config->ip_memoria;
		interfaz->puerto_memoria = config->puerto_memoria;
		break;
	case DIALFS:
		interfaz->tipo_interfaz = DIALFS;
		interfaz->ip_memoria = config->ip_memoria;
		interfaz->puerto_memoria = config->puerto_memoria;
		interfaz->path_base_dialfs = config->path_base_dialfs;
		interfaz->block_size = config->block_size;
		interfaz->block_count = config->block_count;
		interfaz->retraso_compactacion = config->retraso_compactacion;
		break;
	default:
		return NULL;
	}
	return interfaz;
}

enum_interfaz asignar_interfaz(char *nombre_Interfaz)
{
	if (strcmp(nombre_Interfaz, "GENERICA") == 0)
	{
		return GENERICA;
	}
	if (strcmp(nombre_Interfaz, "STDIN") == 0)
	{
		return STDIN;
	}
	if (strcmp(nombre_Interfaz, "STDOUT") == 0)
	{
		return STDOUT;
	}
	if (strcmp(nombre_Interfaz, "DIALFS") == 0)
	{
		return DIALFS;
	}
	return -1;
}

void inicializar_config_IO(char *PATH)
{
	config_interfaz = malloc(sizeof(config_io));

	char directorioActual[2048];
	getcwd(directorioActual, sizeof(directorioActual));
	char *ultimo_dir = basename(directorioActual);
	if (strcmp(ultimo_dir, "bin") == 0)
	{
		chdir("..");
		getcwd(directorioActual, sizeof(directorioActual));
	}
	else
	{
		getcwd(directorioActual, sizeof(directorioActual));
	}
	strcat(directorioActual, PATH);

	config_interfaz->config = iniciar_config(PATH);
	config_interfaz->tipo_interfaz = config_get_string_value(config_interfaz->config, "TIPO_INTERFAZ");
	config_interfaz->ip_kernel = config_get_string_value(config_interfaz->config, "IP_KERNEL");
	config_interfaz->puerto_kernel = atoi(config_get_string_value(config_interfaz->config, "PUERTO_KERNEL"));

	if (strcmp(config_interfaz->tipo_interfaz, "GENERICA") == 0)
	{
		config_interfaz->tiempo_unidad_trabajo = config_get_int_value(config_interfaz->config, "TIEMPO_UNIDAD_TRABAJO");
	}
	if (strcmp(config_interfaz->tipo_interfaz, "STDIN") == 0)
	{
		config_interfaz->ip_memoria = config_get_string_value(config_interfaz->config, "IP_MEMORIA");
		config_interfaz->puerto_memoria = atoi(config_get_string_value(config_interfaz->config, "PUERTO_MEMORIA"));
	}
	if (strcmp(config_interfaz->tipo_interfaz, "STDOUT") == 0)
	{
		config_interfaz->tiempo_unidad_trabajo = config_get_int_value(config_interfaz->config, "TIEMPO_UNIDAD_TRABAJO");
		config_interfaz->ip_memoria = config_get_string_value(config_interfaz->config, "IP_MEMORIA");
		config_interfaz->puerto_memoria = atoi(config_get_string_value(config_interfaz->config, "PUERTO_MEMORIA"));
	}
	if (strcmp(config_interfaz->tipo_interfaz, "DIALFS") == 0)
	{
		config_interfaz->tiempo_unidad_trabajo = config_get_int_value(config_interfaz->config, "TIEMPO_UNIDAD_TRABAJO");
		config_interfaz->ip_memoria = config_get_string_value(config_interfaz->config, "IP_MEMORIA");
		config_interfaz->puerto_memoria = atoi(config_get_string_value(config_interfaz->config, "PUERTO_MEMORIA"));
		config_interfaz->path_base_dialfs = config_get_string_value(config_interfaz->config, "PATH_BASE_DIALFS");
		config_interfaz->block_size = config_get_int_value(config_interfaz->config, "BLOCK_SIZE");
		config_interfaz->block_count = config_get_int_value(config_interfaz->config, "BLOCK_COUNT");
		config_interfaz->retraso_compactacion = config_get_int_value(config_interfaz->config, "RETRASO_COMPACTACION");
	}
}

void realizar_operacion(t_interfaz *interfaz)
{
	estoy_libre = 0;
	switch (interfaz->tipo_interfaz)
	{
	case GENERICA:
		realizar_operacion_gen(interfaz);
		break;
	case STDIN:
		realizar_operacion_stdin(interfaz);
		break;
	case STDOUT:
		realizar_operacion_stdout(interfaz);
		break;
	case DIALFS:
		realizar_operacion_dialfs(interfaz);
		break;
	default:
		log_error(logger, "ERROR - TIPO INTERFAZ DESCONOCIDA");
		break;
	}
}