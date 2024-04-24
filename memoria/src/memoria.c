#include "../include/memoria.h"
#include <stdint.h>
#include "utils/include/instrucciones.h"

int main(int argc, char *argv[])
{

	logger = iniciar_logger("memoria.log", "MEMORIA");
	struct config_memoria *valores_config = config_memoria();

	// LEVANTAMOS EL SERVIDOR DE MEMORIA
	levantarServidor(logger, valores_config->puerto_memoria, valores_config->ip_memoria, "SERVIDOR MEMORIA");

	// terminar_programa(NULL, logger, config_memoria);
	
	destruirConfig(valores_config->config);
	destruirLog(logger);

}
void enviarInstruccion(char *pathArch){
 mensaje_cpu_kernel codigoCPU = recibir_codigo(socket_cpu);
 if(codigoCPU == PEDIDO_INSTRUCCION){

	t_list*listaInstrucciones= pasajeDeArchivoAListaInstrucciones(pathArch);


 }

}
instruccion* crearInstruccion(char* linea) {
   instruccion* instruccion = malloc(sizeof(instruccion));
    char* token = strtok(linea, " "); // El primer token es el código de la instrucción.
    instruccion->codigo = strdup(token);

    return instruccion;
}
t_list * pasajeDeArchivoAListaInstrucciones(char * pathArch){
	t_list*listInstrucciones= list_create(); //creo el puntero a la lista
	FILE * arch = fopen(pathArch, "r");
	if(arch == NULL){
		perror("Error en abrir el archivo");//me fijo si se pudo abrir el archivo
	}
	char * linea ; //este seria el buffer para ir leyendo el archivo
	while(fgets(linea, sizeo(linea), arch)){ // voy leyendo el archivo
		strtok(linea, "\n");
		instruccion*unaInstruccion = crearInstruccion(linea);
		list_add(listInstrucciones, unaInstruccion); //agrego unaInstruccion a la lista
	}
}
uint32_t recibir_codigo(int unSocket ){
    int codigo;
	ssize_t unResultado =recv(socket, &codigo, sizeof(int), MSG_WAITALL);
	if(unResultado ==-1){
		perror("Error con el  recv");
		exit(EXIT_FAILURE);
	}else{
		return codigo;
	}

}

