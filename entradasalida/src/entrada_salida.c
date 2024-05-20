#include "../include/entrada_salida.h"

t_interfaz *interfaz_io;
config_io *valores_config;
extern sem_t *sem_kernel_io_generica;
int main(int argc, char *argv[])
{

	logger = iniciar_logger("entrada_salida.log", "ENTRADA_SALIDA");
	// config_io *valores_config = inicializar_config_IO("entrada_salida.config");
	// conexion_memoria_desde_IO = levantarCliente(logger, "Servidor Memoria", valores_config->ip_memoria, valores_config->puerto_memoria, "ENTRADA/SALIDA se conecta a Memoria");
	// conexion_kernel = levantarCliente(logger, "Servidor Kernel", valores_config->ip_kernel, valores_config->puerto_kernel, "ENTRADA/SALIDA se conecta a Kernel");
	// esto previo no va, ya que cada interfaz se va a prender una vez que se levanta en el sistema

	// ACA ARRANCA EL PROGRAMA

	char *nombre_interfaz = readline("Ingrese el nombre de la interfaz: ");
	char *path_configuracion = readline("Ingrese PATH con la configuracion de la interfaz: ");
	//  LEVANTAR INTERFAZ
	levantar_interfaz(nombre_interfaz, path_configuracion); // recibo el nombre id de la interfaz y su archivo de configuracion

	// esto no iria
	// terminar_programa(conexion_memoria_desde_IO, logger, valores_config->config);
	// liberarConexion(conexion_kernel);
	/* creo de abse 4 instancias de los tipos de interfaces que existen, y luego dependiendo el tipo que se vaya a crear, luego borro los otros*/
	free(nombre_interfaz);
	free(path_configuracion);
	log_destroy(logger);
	config_destroy(valores_config);
}

// FUNCIONES
void levantar_interfaz(char *nombre, char *PATH)
{
	// primero leo el path y obtengo el tipo de interfaz luego creo segun el tipo de interfaz
	// aca solo inicializo el archivo de config
	config_io *config_interfaz = inicializar_config_IO(PATH);
	estoy_libre = 1;
	switch (asignar_interfaz(config_interfaz->tipo_interfaz))
	{
	// aca creo segun el tipo de interfaz con su config
	case GENERICA:
		interfaz_io = crear_interfaz_generica(config_interfaz);
		arrancar_interfaz_generica(interfaz_io);
		break;
	case STDIN:
		interfaz_io = crear_interfaz_stdin(config_interfaz);
		arrancar_interfaz_stdin(interfaz_io);
		break;
	case STDOUT:
		interfaz_io = crear_interfaz_stdout(config_interfaz);
		arrancar_interfaz_stdout(interfaz_io);
		break;
	case DIALFS:
		interfaz_io = crear_interfaz_dialfs(config_interfaz);
		arrancar_interfaz_dialfs(interfaz_io);
		break;
	}
}

t_interfaz *crear_interfaz_generica(config_io *config)
{
	t_interfaz *interfaz;
	interfaz = malloc(sizeof(t_interfaz));
	interfaz->tipo_interfaz = asignar_interfaz(config->tipo_interfaz);
	interfaz->tiempo_unidad_trabajo = config->tiempo_unidad_trabajo;
	interfaz->ip_kernel = config->ip_kernel;
	interfaz->puerto_kernel = config->puerto_kernel;
	return interfaz;
}

t_interfaz *crear_interfaz_stdin(config_io *config)
{
	t_interfaz *interfaz = malloc(sizeof(t_interfaz));
	interfaz->tipo_interfaz = asignar_interfaz(config->tipo_interfaz);
	interfaz->ip_kernel = config->ip_kernel;
	interfaz->puerto_kernel = config->puerto_kernel;
	interfaz->ip_memoria = config->ip_memoria;
	interfaz->puerto_memoria = config->puerto_memoria;
	return interfaz;
}

t_interfaz *crear_interfaz_stdout(config_io *config)
{
	t_interfaz *interfaz;
	interfaz = malloc(sizeof(t_interfaz));
	interfaz->tipo_interfaz = asignar_interfaz(config->tipo_interfaz);
	interfaz->tiempo_unidad_trabajo = config->tiempo_unidad_trabajo;
	interfaz->ip_kernel = config->ip_kernel;
	interfaz->puerto_kernel = config->puerto_kernel;
	interfaz->ip_memoria = config->ip_memoria;
	interfaz->puerto_memoria = config->puerto_memoria;
	return interfaz;
}

t_interfaz *crear_interfaz_dialfs(config_io *config)
{
	t_interfaz *interfaz;
	interfaz = malloc(sizeof(t_interfaz));
	interfaz->tipo_interfaz = asignar_interfaz(config->tipo_interfaz);
	interfaz->tiempo_unidad_trabajo = config->tiempo_unidad_trabajo;
	interfaz->ip_kernel = config->ip_kernel;
	interfaz->puerto_kernel = config->puerto_kernel;
	interfaz->ip_memoria = config->ip_memoria;
	interfaz->puerto_memoria = config->puerto_memoria;
	interfaz->path_base_dialfs = config->path_base_dialfs;
	interfaz->block_size = config->block_size;
	interfaz->block_count = config->block_count;
	interfaz->retraso_compactacion = config->retraso_compactacion;
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
	return -1; // no encontro la interfaz
}

config_io *inicializar_config_IO(char *PATH)
{
	valores_config = malloc(sizeof(config_io));

	// creo el config
	valores_config->config = iniciar_config(PATH);
	// configuraciones basicas
	valores_config->tipo_interfaz = config_get_string_value(valores_config->config, "TIPO_INTERFAZ");
	valores_config->ip_kernel = config_get_string_value(valores_config->config, "IP_KERNEL");
	valores_config->puerto_kernel = atoi(config_get_string_value(valores_config->config, "PUERTO_KERNEL"));
	if (strcmp(valores_config->tipo_interfaz, "GENERICA") == 0)
	{
		valores_config->tiempo_unidad_trabajo = config_get_int_value(valores_config->config, "TIEMPO_UNIDAD_TRABAJO");
	}
	if (strcmp(valores_config->tipo_interfaz, "STDIN") == 0)
	{
		valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP_MEMORIA");
		valores_config->puerto_memoria = atoi(config_get_string_value(valores_config->config, "PUERTO_MEMORIA"));
	}
	if (strcmp(valores_config->tipo_interfaz, "STDOUT") == 0)
	{
		valores_config->tiempo_unidad_trabajo = config_get_int_value(valores_config->config, "TIEMPO_UNIDAD_TRABAJO");
		valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP_MEMORIA");
		valores_config->puerto_memoria = atoi(config_get_string_value(valores_config->config, "PUERTO_MEMORIA"));
	}
	if (strcmp(valores_config->tipo_interfaz, "DIALFS") == 0)
	{
		valores_config->tiempo_unidad_trabajo = config_get_int_value(valores_config->config, "TIEMPO_UNIDAD_TRABAJO");
		valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP_MEMORIA");
		valores_config->puerto_memoria = atoi(config_get_string_value(valores_config->config, "PUERTO_MEMORIA"));
		valores_config->path_base_dialfs = config_get_string_value(valores_config->config, "PATH_BASE_DIALFS");
		valores_config->block_size = config_get_int_value(valores_config->config, "BLOCK_SIZE");
		valores_config->block_count = config_get_int_value(valores_config->config, "BLOCK_COUNT");
		valores_config->retraso_compactacion = config_get_int_value(valores_config->config, "RETRASO_COMPACTACION");
	}
	return valores_config;
}

void arrancar_interfaz_generica(t_interfaz *interfaz_io)
{
	int unidad_tiempo = interfaz_io->tiempo_unidad_trabajo;
	char *ip = interfaz_io->ip_kernel;
	int puerto = interfaz_io->puerto_kernel;
	// 1-conectar con kernel
	conexion_kernel = levantarCliente(logger, "KERNEL", ip, string_itoa(puerto), "Hola, soy una interfaz GENERICA");
	// 2-esperar que kernel envie un mensaje
	while (1)
	{
		// 3-atender el mensaje que envia kernel
		op_code consulta_kernel = recibir_operacion(conexion_kernel);
		sem_wait(sem_kernel_io_generica); // espero a que el kernel me avise que envio algo
		if (consulta_kernel == CONSULTAR_DISPONIBILDAD)
		{
			if (estoy_libre) // DE MOMENTO IMPLEMENTO CON INTS, ES MEDIO FEO,TALVEZ PODEMOS IMPLEMENTAR SEMAFOROS
			{
				enviar_cod_enum(conexion_kernel, ESTOY_LIBRE);
				estoy_libre = 0;
				realizar_operacion_gen();
				// 4-responder al kernel que termine
				enviar_cod_enum(conexion_kernel, CONCLUI_OPERACION);
			}
			else
			{
				// si no estoy libre ... IMPLEMENTAR
				enviar_cod_enum(conexion_kernel, NO_ESTOY_LIBRE);
			}
		}
		else
		{
			log_info(logger, "No comprendo la instruccion que me envias");
		}
		// 5-vuelvo al paso 2
	}
}
void realizar_operacion_gen()
{
	tipo_buffer *buffer_sol_operacion = crear_buffer();
	buffer_sol_operacion = recibir_buffer(socket_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
	int unidades_tiempo = leer_buffer_enteroUint32(buffer_sol_operacion);
	if (sol_operacion == IO_GEN_SLEEP)
	{
		sleep(unidades_tiempo);
	}
	else
	{
		log_info(logger, "Hubo un error al traer la operacion IO_GEN_SLEEP");
	}
	estoy_libre = 1;
	destruir_buffer(buffer_sol_operacion);
}

void arrancar_interfaz_stdin(t_interfaz *interfaz_io)
{
	int unidad_tiempo = interfaz_io->tiempo_unidad_trabajo;
	char *ip = interfaz_io->ip_kernel;
	int puerto = interfaz_io->puerto_kernel;
	// 1-conectar con kernel
	conexion_kernel = levantarCliente(logger, "KERNEL", ip, puerto, "Hola, soy una interfaz STDIN");
	// 2-esperar que kernel envie un mensaje wait()
	while (1)
	{
		// 3-atender el mensaje que envia kernel
		// talvez vaya uyn semaoforo aca
		op_code consulta_kernel = recibir_operacion(conexion_kernel);
		if (consulta_kernel == CONSULTAR_DISPONIBILDAD)
		{
			if (estoy_libre) // DE MOMENTO IMPLEMENTO CON INTS, ES MEDIO FEO,TALVEZ PODEMOS IMPLEMENTAR SEMAFOROS
			{
				enviar_cod_enum(conexion_kernel, ESTOY_LIBRE);
				estoy_libre = 0;
				realizar_operacion_stdin();
				// 4-responder al kernel que termine
				enviar_cod_enum(conexion_kernel, CONCLUI_OPERACION);
			}
			else
			{
				// si no estoy libre ... IMPLEMENTAR
				enviar_cod_enum(conexion_kernel, NO_ESTOY_LIBRE);
			}
		}
		else
		{
			log_info(logger, "No comprendo la instruccion que me envias");
		}
		// 5-vuelvo al paso 2
	}
}
void realizar_operacion_stdin()
{
	tipo_buffer *buffer_sol_operacion = crear_buffer();
	buffer_sol_operacion = recibir_buffer(socket_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);

	void *valor_ingresado;
}
void arrancar_interfaz_stdout(t_interfaz *interfaz_io) {}
void arrancar_interfaz_dialfs(t_interfaz *interfaz_io) {}