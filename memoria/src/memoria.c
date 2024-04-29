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
void enviarInstruccion(char *pathArch)
{
	mensaje_cpu_kernel codigoCPU = recibir_codigo(socket_cpu);
	if (codigoCPU == PEDIDO_INSTRUCCION)
	{ /*
		t_buffer *buffer = recibir_buffer(socket_cpu);

		int pid = leer_buffer(buffer);
		int pc = leer_buffer(buffer);
		destruir_buffer(buffer);
		t_pcb*proceso= buscarPCBEnColaPorPid(pid);


		t_list *listaInstrucciones = list_get(proceso->cde->instrucciones);
		buffer = crear_buffer();
		escribir_buffer(buffer, instruccion);
		enviar_buffer(buffer, socket_cpu);
		destruir_buffer();

*/
		/*ENVIAR LA INSTRUCCION  A CPU CON EL SOCKET_CPU
		*/

		
	}
}
/* Dejo estas dos funciones que hicimos en kernel, tambien la vamos a necesitar en memoria t_pcb *buscarProceso(uint32_t pid)
{
    t_pcb *pcb_buscada = NULL;
    colaEstado *colas[] = {cola_new_global, cola_ready_global, cola_exec_global, cola_exit_global}; // Hace vector de colas

    for (int i = 0; i < sizeof(colas) / sizeof(colas[0]); i++) // va fijandose si en cada posicion del vector esta ese pid
    {

        if ((pcb_buscada = buscarPCBEnColaPorPid(pid, colas[i]->estado, colas[i]->nombreEstado)) != NULL) // busqueda
        {
            // pcb_buscada = buscarPCBEnColaPorPid(pid, colas[i]->estado, colas[i]->nombreEstado);
            return pcb_buscada;
            break;
        }
    }
    if (pcb_buscada == NULL)
    {
        printf("No se pudo encontrar ningun PCB asociado al PID %u\n", pid);
    }

    return pcb_buscada;
}

t_pcb *buscarPCBEnColaPorPid(int pid_buscado, t_queue *cola, char *nombreCola)
{

    t_pcb *pcb_buscada;

    // Verificar si la lista está vacía
    if (queue_is_empty(cola))
    {
        printf("El estado %s está vacío.\n", nombreCola);
        return NULL;
    }

    t_queue *colaAux = queue_create();
    // Copiar los elementos a una cola auxiliar y mostrarlos

    while (!queue_is_empty(cola)) // vacia la cola y mientras buscar el elemento
    {
        t_pcb *pcb = queue_pop(cola);

        if (pcb->cde->pid == pid_buscado)
        {
            pcb_buscada = pcb;
        }

        queue_push(colaAux, pcb_buscada);
    }

    // Restaurar la cola original
    while (!queue_is_empty(colaAux))
    {
        queue_push(cola, queue_pop(colaAux));
    }

    // Liberar memoria de la cola auxiliar y sus elementos
    while (!queue_is_empty(colaAux))
    {
        free(queue_pop(colaAux));
    }
    queue_destroy(colaAux);

    if (pcb_buscada != NULL)
    {
        if (pcb_buscada->cde != NULL)
        {
            printf("Se encontró el proceso PID: %i en %s \n", pcb_buscada->cde->pid, nombreCola);
        }
        else
        {
            printf("El PCB encontrado no tiene un puntero válido a cde.\n");
        }
    }
    else
    {
        printf("No se encontró el proceso en %s.\n", nombreCola);
    }

    return pcb_buscada;
}
*/
t_instruccion *crearInstruccion(char *linea)
{
	t_instruccion *instruccion = malloc(sizeof(instruccion));
	char *token = strtok(linea, " "); // El primer token es el código de la instrucción.
	instruccion->codigo = strdup(token);

	return instruccion;
}
t_list *pasajeDeArchivoAListaInstrucciones(char *pathArch)
{
	t_list *listInstrucciones = list_create(); // creo el puntero a la lista
	FILE *arch = fopen(pathArch, "r");
	if (arch == NULL)
	{
		perror("Error en abrir el archivo"); // me fijo si se pudo abrir el archivo
	}
	char *linea; // este seria el buffer para ir leyendo el archivo
	while (fgets(linea, sizeo(linea), arch))
	{ // voy leyendo el archivo
		strtok(linea, "\n");
		t_instruccion *unaInstruccion = crearInstruccion(linea);
		list_add(listInstrucciones, unaInstruccion); // agrego unaInstruccion a la lista
	}
}
