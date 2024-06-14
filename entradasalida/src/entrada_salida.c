#include "../include/entrada_salida.h"

t_interfaz *interfaz_io;
config_io *valores_config;

int main(int argc, char *argv[])
{

	logger = iniciar_logger("entrada_salida.log", "ENTRADA_SALIDA");

	char *nombre_interfaz = readline("Ingrese el nombre de la interfaz: ");
	char *path_configuracion = readline("Ingrese el nombre del archivo con la configuracion de la interfaz: ");
	strcat(path_configuracion, ".config");
	levantar_interfaz(nombre_interfaz, path_configuracion);

	free(nombre_interfaz);
	free(path_configuracion);
	log_destroy(logger);
	config_destroy(valores_config);
}

void levantar_interfaz(char *nombre, char *PATH)
{
	config_io *config_interfaz = inicializar_config_IO(PATH);
	estoy_libre = 1;
	switch (asignar_interfaz(config_interfaz->tipo_interfaz))
	{
	case GENERICA:
		interfaz_io = crear_interfaz_generica(config_interfaz, nombre);
		arrancar_interfaz_generica(interfaz_io);
		break;
	case STDIN:
		interfaz_io = crear_interfaz_stdin(config_interfaz, nombre);
		arrancar_interfaz_stdin(interfaz_io);
		break;
	case STDOUT:
		interfaz_io = crear_interfaz_stdout(config_interfaz, nombre);
		arrancar_interfaz_stdout(interfaz_io);
		break;
	case DIALFS:
		interfaz_io = crear_interfaz_dialfs(config_interfaz, nombre);
		arrancar_interfaz_dialfs(interfaz_io);
		break;
	}
}

t_interfaz *crear_interfaz_generica(config_io *config, char *nombre)
{
	t_interfaz *interfaz;
	interfaz = malloc(sizeof(t_interfaz));
	interfaz->tipo_interfaz = asignar_interfaz(config->tipo_interfaz);
	interfaz->tiempo_unidad_trabajo = config->tiempo_unidad_trabajo;
	interfaz->ip_kernel = config->ip_kernel;
	interfaz->puerto_kernel = config->puerto_kernel;
	interfaz->nombre_interfaz = nombre;
	return interfaz;
}

t_interfaz *crear_interfaz_stdin(config_io *config, char *nombre)
{
	t_interfaz *interfaz = malloc(sizeof(t_interfaz));
	interfaz->tipo_interfaz = asignar_interfaz(config->tipo_interfaz);
	interfaz->ip_kernel = config->ip_kernel;
	interfaz->puerto_kernel = config->puerto_kernel;
	interfaz->ip_memoria = config->ip_memoria;
	interfaz->puerto_memoria = config->puerto_memoria;
	interfaz->nombre_interfaz = nombre;
	return interfaz;
}

t_interfaz *crear_interfaz_stdout(config_io *config, char *nombre)
{
	t_interfaz *interfaz;
	interfaz = malloc(sizeof(t_interfaz));
	interfaz->tipo_interfaz = asignar_interfaz(config->tipo_interfaz);
	interfaz->tiempo_unidad_trabajo = config->tiempo_unidad_trabajo;
	interfaz->ip_kernel = config->ip_kernel;
	interfaz->puerto_kernel = config->puerto_kernel;
	interfaz->ip_memoria = config->ip_memoria;
	interfaz->puerto_memoria = config->puerto_memoria;
	interfaz->nombre_interfaz = nombre;
	return interfaz;
}

t_interfaz *crear_interfaz_dialfs(config_io *config, char *nombre)
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
	interfaz->nombre_interfaz = nombre;
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
	// 1-conectar con kernel
	conexion_kernel = levantarCliente(logger, "KERNEL", interfaz_io->ip_kernel, string_itoa(interfaz_io->puerto_kernel));
	tipo_buffer *buffer_kernel_io = crear_buffer();

	enviar_cod_enum(conexion_kernel, SOLICITUD_CONEXION_IO);
	char *nombre_interfaz = interfaz_io->nombre_interfaz;
	agregar_buffer_para_enterosUint32(buffer_kernel_io, interfaz_io->tipo_interfaz);
	agregar_buffer_para_string(buffer_kernel_io, nombre_interfaz);

	enviar_buffer(buffer_kernel_io, conexion_kernel);
	destruir_buffer(buffer_kernel_io);
	// 2-esperar que kernel envie un mensaje
	op_code mensaje_kernel = recibir_operacion(conexion_kernel);
	if (mensaje_kernel == ESTABA_CONECTADO)
	{
		log_info(logger, "Ya estoy conectada al Kernel");
		exit(1);
	}
	else if (mensaje_kernel == NO_ESTABA_CONECTADO)
	{
		log_info(logger, "Conexion a Kernel Correcta");
	}
	while (1)
	{
		op_code consulta_kernel = recibir_operacion(conexion_kernel);
		// 3-atender el mensaje que envia kernel
		if (consulta_kernel == CONSULTAR_DISPONIBILDAD)
		{
			if (estoy_libre)
			{
				enviar_cod_enum(conexion_kernel, ESTOY_LIBRE);
				estoy_libre = 0;
				realizar_operacion_gen();
				// 4-responder al kernel que termine
				enviar_cod_enum(conexion_kernel, CONCLUI_OPERACION);
			}
			else
			{
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
	tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
	int unidades_tiempo = leer_buffer_enteroUint32(buffer_sol_operacion);
	int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
	if (sol_operacion == IO_GEN_SLEEP)
	{
		sleep(unidades_tiempo);
		log_info(logger, "PID: <%d> - Operacion: <IO_GEN_SLEEP>", pid);
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
	// 1-conectar con kernel y memoria
	conexion_kernel = levantarCliente(logger, "KERNEL", interfaz_io->ip_kernel, string_itoa(interfaz_io->puerto_kernel));
	conexion_memoria = levantarCliente(logger, "MEMORIA", interfaz_io->ip_memoria, string_itoa(interfaz_io->puerto_memoria));

	tipo_buffer *buffer_kernel_io = crear_buffer();

	enviar_cod_enum(conexion_kernel, SOLICITUD_CONEXION_IO);
	char *nombre_interfaz = interfaz_io->nombre_interfaz;
	agregar_buffer_para_enterosUint32(buffer_kernel_io, interfaz_io->tipo_interfaz);
	agregar_buffer_para_string(buffer_kernel_io, nombre_interfaz);

	enviar_buffer(buffer_kernel_io, conexion_kernel);
	destruir_buffer(buffer_kernel_io);
	// 2-esperar que kernel envie un mensaje
	op_code mensaje_kernel = recibir_operacion(conexion_kernel);
	if (mensaje_kernel == ESTABA_CONECTADO)
	{
		log_info(logger, "Ya estoy conectada al Kernel");
		exit(1);
	}
	else if (mensaje_kernel == NO_ESTABA_CONECTADO)
	{
		log_info(logger, "Conexion a Kernel Correcta");
	}

	while (1)
	{
		op_code consulta_kernel = recibir_operacion(conexion_kernel);
		// 3-atender el mensaje que envia kernel
		if (consulta_kernel == CONSULTAR_DISPONIBILDAD)
		{
			if (estoy_libre)
			{
				enviar_cod_enum(conexion_kernel, ESTOY_LIBRE);
				estoy_libre = 0;
				realizar_operacion_stdin();
				// 4-responder al kernel que termine
				enviar_cod_enum(conexion_kernel, CONCLUI_OPERACION);
			}
			else
			{
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
	tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
	int limitante_cadena = leer_buffer_enteroUint32(buffer_sol_operacion); // con este valor, limito la cadena que meto
	// ej si meto la cadena, Interfaz y el limite es de 3 bytes, entonces solo puedo meter Int
	int direccion_fisica = leer_buffer_enteroUint32(buffer_sol_operacion); // donde voy a grabar el dato en memoria
	int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
	if (sol_operacion == IO_STDIN_READ)
	{
		char *texto_ingresado = readline("Ingrese un texto por teclado: ");

		tipo_buffer *buffer_a_memoria = crear_buffer();

		agregar_buffer_para_enterosUint32(buffer_a_memoria, direccion_fisica);
		agregar_buffer_para_enterosUint32(buffer_a_memoria, pid);
		agregar_buffer_para_enterosUint32(buffer_a_memoria, limitante_cadena);
		agregar_buffer_para_string(buffer_a_memoria, texto_ingresado);

		enviar_cod_enum(conexion_memoria, ACCESO_ESPACIO_USUARIO);
		enviar_cod_enum(conexion_memoria, PEDIDO_ESCRITURA);
		enviar_cod_enum(conexion_memoria, SOLICITUD_INTERFAZ_STDIN);
		enviar_buffer(buffer_a_memoria, conexion_memoria);

		destruir_buffer(buffer_a_memoria);

		op_code codigo_memoria = recibir_operacion(conexion_memoria);
		if (codigo_memoria == OK)
		{
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

void arrancar_interfaz_stdout(t_interfaz *interfaz_io)
{
	// 1-conectar con memoria
	conexion_kernel = levantarCliente(logger, "KERNEL", interfaz_io->ip_kernel, string_itoa(interfaz_io->puerto_kernel));
	conexion_memoria = levantarCliente(logger, "MEMORIA", interfaz_io->ip_memoria, string_itoa(interfaz_io->puerto_memoria));

	tipo_buffer *buffer_kernel_io = crear_buffer();

	enviar_cod_enum(conexion_kernel, SOLICITUD_CONEXION_IO);
	char *nombre_interfaz = interfaz_io->nombre_interfaz;
	agregar_buffer_para_enterosUint32(buffer_kernel_io, interfaz_io->tipo_interfaz);
	agregar_buffer_para_string(buffer_kernel_io, nombre_interfaz);

	enviar_buffer(buffer_kernel_io, conexion_kernel);
	destruir_buffer(buffer_kernel_io);
	// 2-esperar que kernel envie un mensaje
	op_code mensaje_kernel = recibir_operacion(conexion_kernel);
	if (mensaje_kernel == ESTABA_CONECTADO)
	{
		log_info(logger, "Ya estoy conectada al Kernel");
		exit(1);
	}
	else if (mensaje_kernel == NO_ESTABA_CONECTADO)
	{
		log_info(logger, "Conexion a Kernel Correcta");
	}
	while (1)
	{
		op_code consulta_kernel = recibir_operacion(conexion_kernel);
		// 3-atender el mensaje que envia kernel
		if (consulta_kernel == CONSULTAR_DISPONIBILDAD)
		{
			if (estoy_libre)
			{
				enviar_cod_enum(conexion_kernel, ESTOY_LIBRE);
				estoy_libre = 0;
				realizar_operacion_stdout();
				// 4-responder al kernel que termine
				enviar_cod_enum(conexion_kernel, CONCLUI_OPERACION);
			}
			else
			{
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

void realizar_operacion_stdout()
{
	tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
	int limitante_cadena = leer_buffer_enteroUint32(buffer_sol_operacion); // con este valor, se lo envio a la memoria para que
	// solo lea una cierta cantidad
	int direccion_fisica = leer_buffer_enteroUint32(buffer_sol_operacion); // donde voy a pedirle a memoria que busque el dato
	int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
	if (sol_operacion == IO_STDOUT_WRITE)
	{
		tipo_buffer *buffer_a_memoria = crear_buffer();

		agregar_buffer_para_enterosUint32(buffer_a_memoria, direccion_fisica);
		agregar_buffer_para_enterosUint32(buffer_a_memoria, pid);
		agregar_buffer_para_enterosUint32(buffer_a_memoria, limitante_cadena);

		enviar_cod_enum(conexion_memoria, ACCESO_ESPACIO_USUARIO);
		enviar_cod_enum(conexion_memoria, PEDIDO_LECTURA);
		enviar_cod_enum(conexion_memoria, SOLICITUD_INTERFAZ_STDOUT);
		enviar_buffer(buffer_a_memoria, conexion_memoria);

		destruir_buffer(buffer_a_memoria);

		op_code codigo_memoria = recibir_operacion(socket_memoria);
		if (codigo_memoria == OK)
		{
			tipo_buffer *lectura = recibir_buffer(socket_memoria);
			char *valor = leer_buffer_string(lectura);
			destruir_buffer(lectura);

			log_info(logger, "Valor hallado en Direccion Fisica <%d> : %s", direccion_fisica, valor);
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

void arrancar_interfaz_dialfs(t_interfaz *interfaz_io) // IMPLEMENTAR
{
}

void realizar_operacion_dialfs() // IMPLEMENTAR
{
	// FALTAN ESTOS LOGS
	// log_info(logger, "PID: <%d> - Inicio Compactacion");
	// log_info(logger, "PID: <%d> - FIn Compactacion");
	tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
	t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
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