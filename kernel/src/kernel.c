#include "../include/kernel.h"

int main(int argc, char *argv[])
{

	logger = iniciar_logger("kernel.log", "KERNEL");

	// CONFIG
	struct config_kernel *valores_config = config_kernel();

	// KERNEL COMO SERVER DE I0
	levantarServidor(logger, valores_config->puerto_escucha, valores_config->ip_memoria, "SERVIDOR KERNEL");
	// KERNEL COMO CLIENTE
	levantarCliente(conexion_memoria, logger, "MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "KERNEL SE CONECTO A MEMORIA");
	levantarCliente(conexion_cpu, logger, "CPU", valores_config->ip_cpu, valores_config->puerto_cpu_dispatch, "KERNEL SE CONECTO A CPU");

	// CONSOLA INTERACTIVA
	iniciar_consola_interactiva(logger);

	// TO DO ARREGLAR PROBLEMA CON LOS LOGGERS PARA QUE ANDEN BIEN Y NO DE SEGMETATION FAULT
	terminar_programa(conexion_cpu, logger, valores_config->config);
	terminar_programa(conexion_memoria, logger, valores_config->config);
}

void iniciar_consola_interactiva(t_log *logger)
{

	char *cod_operacion;
	while (1)
	{
		mostrar_operaciones_realizables();
		cod_operacion = readline("Ingrese un valor para realizar una de las siguientes acciones: ");
		ejecutar_operacion(cod_operacion, logger);
		free(cod_operacion);
	}
}

void gestionar_peticiones_memoria()
{
	printf("Gestionar Peticion Memoria");
}

void gestionar_peticiones_interfaces()
{
	printf("Gestionar Peticion Interfaces");
}

void planificar_ejecucion_procesos()
{
	printf("Planificar Ejecucion procesos");
}

void mostrar_operaciones_realizables()
{
	printf("1-Ejecutar Script de Operaciones\n");
	printf("2-Iniciar Proceso\n");
	printf("3-Finalizar Proceso\n");
	printf("4-Iniciar Planificacion\n");
	printf("5-Detener Planificacion\n");
	printf("6-Listar Procesos por Estado\n");
}

void ejecutar_operacion(char *codigo_operacion, t_log *logger)
{
/* 	int cod_op = codigo_operacion;
	// hay que convertir el char a int o algo similar usando cmp
	switch (cod_op)
	{
	case 1:
		ejecutar_script();
		exit(1);
		break;
	case 2:
		iniciar_proceso();
		exit(1);
		break;
	case 3:
		detener_proceso();
		exit(1);
		break;
	case 4:
		iniciar_planificacion();
		exit(1);
		break;
	case 5:
		detener_planificacion();
		exit(1);
		break;
	case 6:
		listar_procesos_x_estado();
		exit(1);
	default:
		printf("No es una opcion valida, ingrese otra vez");
		iniciar_consola_interactiva(logger);
		exit(1);
		break;
	} */
}

void ejecutar_script()
{
	printf("Ejecutar Script");
}
void iniciar_proceso()
{
	printf("Iniciar Proceso");
}
void detener_proceso()
{
	printf("Detener Proceso");
}
void iniciar_planificacion()
{
	printf("Iniciar Planificacion");
}
void detener_planificacion()
{
	printf("Detener Planificacion");
}
void listar_procesos_x_estado()
{
	printf("Listar Procesos Por Estado");
}