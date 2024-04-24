#include <../include/cpu.h>
#include "utils/include/instrucciones.h"
#include "utils/src/serializacion.h"
#include "kernel/include/kernel.h"
#include <string.h>
#include "utils/include/registros.h"
int main(int argc, char *argv[])
{
	logger = iniciar_logger("cpu.log", "CPU");

	// CONFIG
	struct config_cpu *valores_config = config_cpu();
	// Falta que CPU inicie servidor como INTERRUPT y como DISPATCH
	levantarServidor(logger, valores_config->puerto_escucha_dispatch, valores_config->ip_kernel, "SERVIDOR CPU");
	levantarCliente(CONEXION_A_MEMORIA, logger, "SERVIDOR  MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "CPU SE CONECTO A MEMORIA");
	terminar_programa(CONEXION_A_MEMORIA, logger, valores_config->config);



}
void recibirPCB(tipo_buffer*buffer,t_pcb*pcb){
	//Tomo como condiciom que el PCB V a estar cuando me lo pases serializado y yo lo voya  deserializar
	//con la funcion memcpy
   // me pasan el pcb y el buffer
    memcpy(&(pcb->estado), buffer, sizeof(int));
    memcpy(&(pcb->prioridad), buffer + sizeof(int), sizeof(int));
    memcpy(&(pcb->flag_clock), buffer + 2 * sizeof(int), sizeof(int));
    memcpy(&(pcb->fin_q), buffer + 3 * sizeof(int), sizeof(int));

    // Deserialización del campo 'path'
    int path_length;
    memcpy(&path_length, buffer + 4 * sizeof(int), sizeof(int));
    pcb->path = (char*)malloc(path_length + 1);
    memcpy(pcb->path, buffer + 5 * sizeof(int), path_length);
    pcb->path[path_length] = '\0'; // Agregar el carácter nulo al final

}
void pedirInstruccionAMemoria(){
enviar_codigo(socket_memoria, PEDIDO_INSTRUCCION);//Pido la instruccion
tipo_buffer*buffer = crear_buffer();
envio_buffer(buffer,socket_memoria);
destroy_buffer((buffer));
//actualizo el buffer escribiendo
escribo_buffer(envio_buffer, tipo_cde->pid);
escribo_buffer(envio_buffer, tipo_cde->pc);
envio_buffer(buffer, socket_memoria);
tipo_cde->pc++;// actualizo el contexto de ejercicion
destroy_buffer(buffer);
t_buffer* buffer_recibido = recibir_buffer(socket_memoria);
//instruccion_a_ejecutar = buffer_read_instruccion(buffer_recibido);
//destroy_buffer(buffer_recibido);
//instruccion_actualizada = instruccion_a_ejecutar->codigo;
//ejecutar_instruccion(cde, instruccion_a_ejecutar);


}
void escribo_buffer(t_buffer* buffer, uint8_t entero){
	buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));

	memcpy(buffer->stream + buffer->size, &entero, sizeof(uint8_t));
	buffer->size += sizeof(uint8_t);
}






   