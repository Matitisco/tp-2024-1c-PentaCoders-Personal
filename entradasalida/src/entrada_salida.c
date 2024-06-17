#include "../include/entrada_salida.h"

t_interfaz *interfaz_io;
config_io *valores_config;

int main(int argc, char *argv[])
{
	iniciar_modulo_io();
	liberar_modulo_io();
}

void iniciar_modulo_io()
{
	char *nombre_interfaz = readline("Ingrese el nombre de la interfaz: ");
	char *path_configuracion = readline("Ingrese el nombre del archivo con la configuracion de la interfaz (sin '.config'): ");
	logger = iniciar_logger("entrada_salida.log", nombre_interfaz);
	strcat(path_configuracion, ".config");

	levantar_interfaz(nombre_interfaz, path_configuracion);
}

void liberar_modulo_io()
{
	log_destroy(logger);
	config_destroy(valores_config->config);
}

void levantar_interfaz(char *nombre, char *PATH)
{
	config_io *config_interfaz = inicializar_config_IO(PATH);
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

config_io *inicializar_config_IO(char *PATH)
{
	valores_config = malloc(sizeof(config_io));

	valores_config->config = iniciar_config(PATH);
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
		op_code consulta_kernel = recibir_operacion(conexion_kernel);
		if (consulta_kernel == CONSULTAR_DISPONIBILDAD)
		{
			if (estoy_libre)
			{
				enviar_cod_enum(conexion_kernel, ESTOY_LIBRE);
				realizar_operacion(interfaz);
				enviar_cod_enum(conexion_kernel, CONCLUI_OPERACION);
			}
			else
			{
				enviar_cod_enum(conexion_kernel, NO_ESTOY_LIBRE);
			}
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

	enviar_cod_enum(conexion_kernel, SOLICITUD_CONEXION_IO);
	tipo_buffer *buffer_interfaz_kernel = crear_buffer();
	agregar_buffer_para_enterosUint32(buffer_interfaz_kernel, interfaz->tipo_interfaz);
	agregar_buffer_para_string(buffer_interfaz_kernel, interfaz->nombre_interfaz);
	enviar_buffer(buffer_interfaz_kernel, conexion_kernel);
	destruir_buffer(buffer_interfaz_kernel);

	op_code mensaje_kernel = recibir_operacion(conexion_kernel);
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
	conexion_memoria = levantarCliente(logger, "MEMORIA", interfaz_io->ip_memoria, string_itoa(interfaz_io->puerto_memoria));
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
		realizar_operacion_stdin();
		break;
	case STDOUT:
		realizar_operacion_stdout();
		break;
	case DIALFS:
		realizar_operacion_dialfs();
		break;
	default:
		log_error(logger, "ERROR - TIPO INTERFAZ DESCONOCIDA");
		break;
	}
}

void realizar_operacion_gen(t_interfaz *interfaz)
{
	tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
	int unidades_tiempo = leer_buffer_enteroUint32(buffer_sol_operacion);
	int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
	destruir_buffer(buffer_sol_operacion);

	if (sol_operacion == IO_GEN_SLEEP)
	{
		sleep_ms(unidades_tiempo * interfaz->tiempo_unidad_trabajo);
		log_info(logger, "PID: <%d> - Operacion: <IO_GEN_SLEEP>", pid);
	}
	else
	{
		log_error(logger, "ERROR - INSTRUCCION INCORRECTA");
	}

	estoy_libre = 1;
}

void realizar_operacion_stdin()
{
	tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
	int limitante_cadena = leer_buffer_enteroUint32(buffer_sol_operacion); // con este valor, limito la cadena que meto
	int direccion_fisica = leer_buffer_enteroUint32(buffer_sol_operacion);
	int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
	if (sol_operacion == IO_STDIN_READ)
	{
		char *texto_ingresado = readline("Ingrese un texto por teclado: ");
		char *nuevo_texto;
		truncar_valor(&nuevo_texto, texto_ingresado, limitante_cadena);
		log_info(logger, "TEXTO YA LIMITADO: '%s'", nuevo_texto);
		t_list *lista_enteros = convertir_a_numeros(nuevo_texto);

		enviar_cod_enum(conexion_memoria, ACCESO_ESPACIO_USUARIO);
		enviar_cod_enum(conexion_memoria, PEDIDO_ESCRITURA);
		enviar_cod_enum(conexion_memoria, SOLICITUD_INTERFAZ_STDIN);

		tipo_buffer *buffer_a_memoria = crear_buffer();

		agregar_buffer_para_enterosUint32(buffer_a_memoria, direccion_fisica);
		agregar_buffer_para_enterosUint32(buffer_a_memoria, pid);
		// agregar_buffer_para_enterosUint32(buffer_a_memoria, limitante_cadena);
		agregar_buffer_para_enterosUint32(buffer_a_memoria, list_size(lista_enteros)); // cant de caracteres que vamos a escribir
		// agregar_buffer_para_string(buffer_a_memoria, nuevo_texto);
		for (int i = 0; i < list_size(lista_enteros); i++)
		{
			int valor = list_get(lista_enteros, i);
			agregar_buffer_para_enterosUint32(buffer_a_memoria, valor);
		}
		enviar_buffer(buffer_a_memoria, conexion_memoria);

		destruir_buffer(buffer_a_memoria);

		op_code codigo_memoria = recibir_operacion(conexion_memoria);
		if (codigo_memoria == OK)
		{
			sleep_ms(valores_config->tiempo_unidad_trabajo);
			log_info(logger, "PID: <%d> - Operacion: <IO_STDIN_READ>", pid);
		}
		else if (codigo_memoria == ERROR_PEDIDO_ESCRITURA)
		{
			log_error(logger, "PID: <%d> - ERROR Operacion: <IO_STDIN_READ>", pid);
		}
	}
	else
	{
		log_info(logger, "Hubo un error al traer la operacion IO_STDIN_READ");
	}
	estoy_libre = 1;
	destruir_buffer(buffer_sol_operacion);
}

void realizar_operacion_stdout()
{
	tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
	int limitante_cadena = leer_buffer_enteroUint32(buffer_sol_operacion); // con este valor, se lo envio a la memoria para que
	log_info(logger, "TAMANIO A LEER %d", limitante_cadena);
	int direccion_fisica = leer_buffer_enteroUint32(buffer_sol_operacion); // donde voy a pedirle a memoria que busque el dato
	int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
	if (sol_operacion == IO_STDOUT_WRITE)
	{
		enviar_cod_enum(conexion_memoria, ACCESO_ESPACIO_USUARIO);
		enviar_cod_enum(conexion_memoria, PEDIDO_LECTURA);
		enviar_cod_enum(conexion_memoria, SOLICITUD_INTERFAZ_STDOUT);

		tipo_buffer *buffer_a_memoria = crear_buffer();
		agregar_buffer_para_enterosUint32(buffer_a_memoria, direccion_fisica);
		agregar_buffer_para_enterosUint32(buffer_a_memoria, pid);
		agregar_buffer_para_enterosUint32(buffer_a_memoria, limitante_cadena);
		enviar_buffer(buffer_a_memoria, conexion_memoria);

		destruir_buffer(buffer_a_memoria);

		op_code codigo_memoria = recibir_operacion(conexion_memoria);
		if (codigo_memoria == OK)
		{
			char *texto_reconstruido = malloc(limitante_cadena);
			tipo_buffer *lectura = recibir_buffer(conexion_memoria);
			for (int i = 0; i < limitante_cadena; i++)
			{
				int entero_valor = leer_buffer_enteroUint32(lectura);
				int_a_char_y_concatenar_a_string(entero_valor, texto_reconstruido);
				log_info(logger, "Valor hallado en Direccion Fisica <%d> : %s", direccion_fisica, string_itoa(entero_valor));
			}
			log_info(logger, "VALOR: %s", texto_reconstruido);
			destruir_buffer(lectura);

			sleep_ms(valores_config->tiempo_unidad_trabajo);
			log_info(logger, "PID: <%d> - Operacion: <IO_STDOUT_WRITE>", pid);
		}
		else if (codigo_memoria == ERROR_PEDIDO_LECTURA)
		{
			log_error(logger, "PID: <%d> - ERROR Operacion: <IO_STDOUT_WRITE>", pid);
		}
	}
	else
	{
		log_info(logger, "Hubo un error al traer la operacion IO_STDOUT_WRITE");
	}
	estoy_libre = 1;
	destruir_buffer(buffer_sol_operacion);
}

void realizar_operacion_dialfs() // IMPLEMENTAR
{
	// FALTAN ESTOS LOGS
	// log_info(logger, "PID: <%d> - Inicio Compactacion");
	// log_info(logger, "PID: <%d> - FIn Compactacion");
	tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
	sleep(valores_config->tiempo_unidad_trabajo);
	int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
	switch (sol_operacion)
	{
	case IO_FS_CREATE:
		log_info(logger, "PID: <%d> - Operacion: <IO_FS_CREATE>", pid);
		log_info(logger, "PID: <%d> - Crear Archivo: <%s>", pid, "NOMBRE ARCHIVO");
		break;
	case IO_FS_DELETE:
		log_info(logger, "PID: <%d> - Operacion: <IO_FS_DELETE>", pid);
		log_info(logger, "PID: <%d> - Eliminar Archivo: <%s>", pid, "NOMBRE ARCHIVO");
		break;
	case IO_FS_TRUNCATE:
		log_info(logger, "PID: <%d> - Operacion: <IO_FS_TRUNCATE>", pid);
		log_info(logger, "PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%s>", pid, "NOMBRE ARCHIVO", "TAMAÑO");
		break;
	case IO_FS_READ:
		log_info(logger, "PID: <%d> - Operacion: <IO_FS_READ>", pid);
		log_info(logger, "PID: <%d> - Leer Archivo: <%s> - Tamaño a Leer: <%s> - Puntero Archivo: <%s>", pid, "NOMBRE ARCHIVO", "TAMAÑO", "PUNTERO_ARCHIVO");
		break;
	case IO_FS_WRITE:
		log_info(logger, "PID: <%d> - Operacion: <IO_FS_WRITE>", pid);
		log_info(logger, "PID: <%d> - Escribir Archivo: <%s> - Tamaño a Escribir: <%s> - Puntero Archivo: <%s>", pid, "NOMBRE ARCHIVO", "TAMAÑO", "PUNTERO_ARCHIVO");
		break;
	default:
		break;
	}
}

t_list *convertir_a_numeros(char *texto)
{
	int i;
	t_list *lista = list_create();
	for (i = 0; texto[i] != '\0'; i++)
	{
		// Convertir el caracter a su valor ASCII
		int numero = (int)texto[i];
		list_add(lista, numero);
	}
	return lista;
}

void truncar_valor(char **valor_nuevo, char *valor_viejo, int limitante)
{
	*valor_nuevo = malloc(limitante * sizeof(char));

	if (*valor_nuevo == NULL)
	{
		return;
	}

	for (int i = 0; i < limitante; i++)
	{
		(*valor_nuevo)[i] = valor_viejo[i];
	}
	(*valor_nuevo)[limitante] = '\0';
	if (valor_viejo != NULL && valor_viejo[0] != '\0')
	{
		free(valor_viejo);
	}
}

void int_a_char_y_concatenar_a_string(int valor, char *cadena)
{

	char caracter = (char)valor;

	int longitud_cadena = strlen(cadena);
	cadena[longitud_cadena] = caracter;
	cadena[longitud_cadena + 1] = '\0';
}
