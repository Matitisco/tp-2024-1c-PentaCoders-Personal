#include "../include/kernel.h"

int main(int argc, char *argv[])
{

	logger = iniciar_logger("kernel.log", "KERNEL");

	// CONFIG
	config = iniciar_config("kernel.config");
	IP = config_get_string_value(config, "IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	PUERTO_CPU = config_get_string_value(config, "PUERTO_CPU_DISPATCH"); // Este va si no va la parte del dispatch y el interrupt

	// puerto_cpu_dispatch= config_get_string_value(config,"PUERTO_CPU_DISPATCH");
	// puerto_cpu_interrupt= config_get_istring_value(config,"PUERTO_CPU_INTERRUPT");

	// KERNEL COMO SERVER DE I0
	levantarServidor(logger, PUERTO_ESCUCHA, IP, "SERVIDOR KERNEL");
	// KERNEL COMO CLIENTE
	levantarCliente(conexion_memoria, logger, "MEMORIA", IP, PUERTO_MEMORIA, "KERNEL SE CONECTO A MEMORIA");
	levantarCliente(conexion_cpu, logger,"CPU", IP, PUERTO_CPU, "KERNEL SE CONECTO A CPU");

	terminar_programa(conexion_cpu,logger,config);
	terminar_programa(conexion_memoria,logger,config);
}