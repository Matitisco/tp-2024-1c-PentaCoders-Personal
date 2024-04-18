#include <../include/cpu.h>

int main(int argc, char *argv[])
{

	logger = iniciar_logger("cpu.log", "CPU");

	// CONFIG
	config = iniciar_config("cpu.config");
	IP = config_get_string_value(config, "IP_MEMORIA");
	PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
	PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	CONEXION_A_MEMORIA = crear_conexion(logger, "Memoria", IP, PUERTO_MEMORIA);

	// Falta que CPU inicie servidor como INTERRUPT y como DISPATCH

	// CPU COMO SERVER DE KERNEL
	levantarServidor(logger, PUERTO_ESCUCHA_DISPATCH, IP, "SERVIDOR CPU");
	// CPU COMO CLIENTE DE MEMORIA
	levantarCliente(CONEXION_A_MEMORIA, logger, "SERVIDOR  MEMORIA", IP, PUERTO_MEMORIA, "CPU SE CONECTO A MEMORIA");

	terminar_programa(CONEXION_A_MEMORIA, logger, config);
}
