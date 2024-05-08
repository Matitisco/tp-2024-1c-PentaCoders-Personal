#include "../include/cpu.h"
int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");
	config_cpu *valores_config_cpu = config_cpu(); // CONFIG
	
	//CPU es servidor de Kernel (conexion INTERRUPT y DISPATCH) y cliente de Memoria 

	//CPU como server de KERNEL
	//levantarServidor(logger, valores_config->puerto_escucha_dispatch, valores_config->ip_kernel, "SERVIDOR CPU DISPATCH");
	//levantarServidor(logger, valores_config->puerto_escucha_interrupt,valores_config->ip_kernel, "SERVIDOR CPU INTERRUPT");


	servidorDeKernel(valores_config_cpu); // Usado para recibir un PCB
	

	levantarCliente(logger, "SERVIDOR  MEMORIA", valores_config_cpu->ip_memoria, valores_config_cpu->puerto_memoria, "CPU SE CONECTO A MEMORIA");
	

	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config_cpu->config);
}

void servidorDeKernel(struct config_cpu *valores_config_cpu){
	int socket_servidor_dispatch = iniciar_servidor(logger,"SERVIDOR CPU DISPATCH",valores_config_cpu->ip_kernel,valores_config_cpu->puerto_escucha_dispatch);
	log_info(logger, "Servidor: %s listo para recibir al cliente", "SERVIDOR CPU DISPATCH");
	proceso_dispatch(socket_servidor_dispatch); // recibir PCB

    //int cliente_fd = esperar_cliente(logger, "CLIENTE CONECTADO DISPATCH", socket_servidor_dispatch);//espero a que se conecte kernel y recibe el socket 
	
	


	//int socket_servidor_interrupt = iniciar_servidor(logger,"SERVIDOR CPU INTERRUPT",valores_config->ip_kernel,valores_config->puerto_escucha_interrupt);
	
}

void proceso_dispatch(int socket_servidor_dispatch){

    log_info(logger, "Esperando Kernel DISPATCH....");
    int socket_kernel_dispatch = esperar_cliente(logger,"SERVIDOR CPU DISPATCH",socket_servidor_dispatch); 
    log_info(logger, "Se conecto el Kernel por DISPATCH");

    while(1){

        mensaje_kernel_cpu op_code = recibir_operacion(socket_kernel_dispatch);

        tipo_buffer* buffer = recibir_buffer(socket_kernel_dispatch);

        switch(op_code){
            case EJECUTAR_PROCESO:
			
                t_cde* cde_recibido = leer_payload_PCB(buffer);//armar funcion segun se envie por kernel

				//Hay una lista de instrucciones y después hay que leer la lista


                destruir_buffer(buffer);

                pthread_mutex_lock(&mutex_cde_ejecutando);
                pid_ejecutar = cde_recibido->pid;
                pthread_mutex_unlock(&mutex_cde_ejecutando);
                
                solicitar_instruccion(cde_recibido);
				//Liberar memoria de cde_recibido al terminar
                break;
			case INTERRUPT:

				break;
			case DESALOJO:

				break;
			case CDE:

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
typedef struct
{
    uint32_t pid;
    uint32_t pc;
    t_registros *registro;
    t_list *lista_instrucciones;
    char* path;

} t_cde;*/



t_cde* leer_payload_PCB (tipo_buffer* buffer){

	t_cde* cde = malloc(sizeof(t_cde));

	cde->pid =leer_buffer_enteroUint32(buffer); // read para uint32
	cde->pc = leer_buffer_enteroUint32(buffer); // read para uint32
	cde->t_registros= leer_buffer_registros(buffer);// hacer funcion para leer
	//cde->lista_instrucciones = leer_buffer_instrucciones(buffer);
	//cde->path = leer_buffer_string(buffer);

	return cde;
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



//Borrar? Usamos dispatchProceso


void solicitar_instruccion(){
//Pedimos a memoria instruccion

//armamos buffer
//send
//despes recv

	//segun la instruccion que recibimos ejecutamos ciclo de instruccion
	
	//ejecutar_ciclo(instruccion);
}

void ejecutar_ciclo(){
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
/*
typedef struct {
    uint32_t size; // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream; // Payload
} t_buffer;

typedef struct {
    uint8_t codigo_operacion;
    t_buffer* buffer;
} t_paquete;

ssize_t send(int socket, const void *buffer, size_t length <-ESO (Tamaño buffer), int flags);

ssize_t recv(int socket, void *buffer, size_t length <-ESO (Tamaño buffer), int flags);
void *memcpy(void *dest, const void *src, size_t n);


socket es global y ya está definido antes 

t_paquete* paquete = malloc(sizeof(t_paquete));
paquete->buffer = malloc(sizeof(t_buffer));

// Primero recibimos el codigo de operacion
recv(unSocket, &(paquete->codigo_operacion), sizeof(uint8_t), 0);

// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
recv(unSocket, &(paquete->buffer->size), sizeof(uint32_t), 0);
paquete->buffer->stream = malloc(paquete->buffer->size);
recv(unSocket, paquete->buffer->stream, paquete->buffer->size, 0);
*/