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
	// iniciar_consola_interactiva(logger);

	terminar_programa(conexion_cpu, logger, valores_config->config);
	liberarConexion(conexion_memoria);
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