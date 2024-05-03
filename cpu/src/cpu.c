#include "../include/cpu.h"
#include <string.h>
#include "../../utils/include/instrucciones.h"

//#include "../../kernel/include/kernel.h"
#include "../../utils/include/registros.h"


int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");

	// CONFIG
	struct config_cpu *valores_config = config_cpu();
	// Falta que CPU inicie servidor como INTERRUPT y como DISPATCH
	levantarServidor(logger, valores_config->puerto_escucha_dispatch, valores_config->ip_kernel, "SERVIDOR CPU DISPATCH");
	levantarServidor(logger, valores_config->puerto_escucha_interrupt,valores_config->ip_kernel, "SERVIDOR CPU INTERRUPT");
	CONEXION_A_MEMORIA = levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "CPU SE CONECTO A MEMORIA");
	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config->config);
}





struct config_cpu *config_cpu()
{
	struct config_cpu *valores_config = malloc(sizeof(struct config_cpu));

	// creo el config
	valores_config->config = iniciar_config("cpu.config");
	valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP_MEMORIA");
	valores_config->ip_kernel = config_get_string_value(valores_config->config, "IP_KERNEL");
	valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_MEMORIA");
	valores_config->puerto_escucha_dispatch = config_get_string_value(valores_config->config, "PUERTO_ESCUCHA_DISPATCH");

	return valores_config;
}





void recibir_cde(){
	mensaje_kernel_cpu codigo= recibir_operacion(socket_kernel_dispatch);
	tipo_buffer* buffer = recibir_buffer_propio(socket_kernel_dispatch);
	if(codigo == EJECUTAR_PROCESO){
		ejecutarproceso();
	}

}
void ejecutarproceso(){
	return 0;
}

/*
void pedirInstruccionAMemoria()
{
	t_cde *cde;
	enviar_codigo(socket_memoria, PEDIDO_INSTRUCCION); // Pido la instruccion
	tipo_buffer *buffer = crear_buffer();
	envio_buffer(buffer, socket_memoria);
	destroy_buffer((buffer));
	// actualizo el buffer escribiendo
	escribo_buffer(envio_buffer, cde->pid);
	escribo_buffer(envio_buffer, cde->pc);
	envio_buffer(buffer, socket_memoria);
	cde->pc++; // actualizo el contexto de ejercicion
	destroy_buffer(buffer);
	tipo_buffer *otro_buffer= recibir_buffer_propio(socket_memoria);
	//t_instrucion *instruccion_a_ejecutar = leer_buffer(otro_buffer); Falta implementar al funcion leer_buffer
	//destruir_buffer(buffer);
	
}*/


// Contexto de ejecucion
void ejecutarCicloInstruccion(int instruccion, uint32_t PC)
{	tipoInstruccion(instruccion);
	PC++;
	// actualizarle pc a ¿memoria?
}

void tipoInstruccion(int instruccion)
{
	switch (instruccion)
	{
	case SET:
		/* code */
		break;
	case SUM:
		/* code */
		break;
	case SUB:
		/* code */
		break;
	case JNZ:
		/* code */
		break;
	case IO_GEN_SLEEP:
		/* code */
		break;
	default:
		break;
	}
}
