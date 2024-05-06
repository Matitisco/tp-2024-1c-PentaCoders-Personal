#include "../include/cpu.h"
int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");
	struct config_cpu *valores_config = config_cpu();
	// CONFIG
	
	// Falta que CPU inicie servidor como INTERRUPT y como DISPATCH

	//CPU como server de KERNEL
	//levantarServidor(logger, valores_config->puerto_escucha_dispatch, valores_config->ip_kernel, "SERVIDOR CPU DISPATCH");
	//levantarServidor(logger, valores_config->puerto_escucha_interrupt,valores_config->ip_kernel, "SERVIDOR CPU INTERRUPT");


	servidorDeKernel(valores_config);

	levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "CPU SE CONECTO A MEMORIA");
	

	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config->config);
}

void servidorDeKernel(struct config_cpu *valores_config){
	int socket_servidor_dispatch = iniciar_servidor(logger,"SERVIDOR CPU DISPATCH",valores_config->ip_kernel,valores_config->puerto_escucha_dispatch);
	log_info(logger, "Servidor: %s listo para recibir al cliente", "SERVIDOR CPU DISPATCH");
    int cliente_fd = esperar_cliente(logger, "SERVIDOR CPU DISPATCH", socket_servidor_dispatch);//espero a que se conecte kernel
	
	//int socket_servidor_interrupt = iniciar_servidor(logger,"SERVIDOR CPU INTERRUPT",valores_config->ip_kernel,valores_config->puerto_escucha_interrupt);
	proceso_dispatch(socket_servidor_dispatch); 
}

void proceso_dispatch(void* socket_server){

    int socket_servidor_dispatch = (int) (intptr_t) socket_server;
    
    log_info(logger, "Esperando Kernel DISPATCH....");
    socket_kernel_dispatch = esperar_cliente(logger,"SERVIDOR CPU DISPATCH",socket_servidor_dispatch); //En la librería faltaba utils/conexiones.h, ya no hace falta agreagarlo porque se encuentra en instruccionescpu.h
    log_info(logger, "Se conecto el Kernel por DISPATCH");
    
    while(1){
        
		mensaje_kernel_cpu op_code = recibir_cod(socket_kernel_dispatch); // Se recibe de Kernel la operación

        tipo_buffer* buffer = recibir_buffer_propio(socket_kernel_dispatch); // Se recibe de Kernel el PCB
		
        switch(op_code){ 
            case EJECUTAR_PROCESO:
			
                t_cde* cde_recibido = recibir_buffer_propio(buffer);
                liberar_buffer(buffer);

                pthread_mutex_lock(&mutex_cde_ejecutando);
                pid_ejecutar = cde_recibido->pid;
                pthread_mutex_unlock(&mutex_cde_ejecutando);
                
                solicitar_instruccion(cde_recibido);
			
                break;
            default:
                //destruir_buffer_nuestro(buffer);
                log_error(logger, "Codigo de operacion desconocido.");
                log_error(logger, "Finalizando modulo.");
                exit(1);
                break;
        }
    }
}

/*

    EJECUTAR_PROCESO,
	INTERRUPT,
	DESALOJO,
	CDE
*/
/*
	CONEXION_A_MEMORIA = levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "CPU SE CONECTO A MEMORIA");
	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config->config);
*/

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

//Borrar? Usamos dispatchProceso
void recibir_cde(){
	mensaje_kernel_cpu codigo= recibir_cod(socket_kernel_dispatch);
	tipo_buffer* buffer = recibir_buffer_propio(socket_kernel_dispatch);
	if(codigo == EJECUTAR_PROCESO){
		ejecutar_proceso();
	}
}

void solicitar_instruccion(){
	//Pedimos a memoria instruccion
	//ejecutamos proceso
	//ejecutar_proceso();
}

void ejecutar_proceso(){
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
void ejecutarCicloInstruccion (int instruccion, uint32_t PC)
{	
	tipoInstruccion(instruccion);
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