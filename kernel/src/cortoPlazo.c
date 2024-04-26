#include "../include/cortoPlazo.h"
// #include "../include/kernel.h"

/*
TRANSICIONES CORTO PLAZO

READY -> EXEC//Se desata cuando no hay ningun en EXEC
EXEC -> READY // Se desata cuando CPU devuelve el CTE y el motivoDeDesalojo (por Interrupt?)


READY -> EXEC // No se

EXEC -> BLOCK //Cuando se completo la rafaga en CPU y se recibe CTE y mDD (por Dispatch)
BLOCK -> READY // Cuando cumple su operacion en IO

BLOCK -> EXIT // Cuando su operacion IO era lo ultimo pendiente
*/

/*
Los procesos que estén en estado READY serán planificados mediante uno de los siguientes algoritmos:
    -FIFO
    -Round Robin
    -Virtual Round Robin
Una vez seleccionado el siguiente proceso a ejecutar,
se lo transicionará al estado EXEC y se enviará su Contexto
de Ejecución al CPU a través del puerto de dispatch, quedando
a la espera de recibir dicho contexto actualizado después de la ejecución,
junto con un motivo de desalojo por el cual fue desplazado a manejar.
En caso que el algoritmo requiera desalojar al proceso en ejecución,
se enviará una interrupción a través de la conexión de interrupt para forzar el desalojo del mismo.
Al recibir el Contexto de Ejecución del proceso en ejecución,
en caso de que el motivo de desalojo implique replanificar se seleccionará
el siguiente proceso a ejecutar según indique el algoritmo. Durante este período la CPU se quedará esperando el nuevo contexto.

*/
/*
t_queue *estado_new;
t_list *estado_ready;
t_list *estado_exec;
t_list *estado_exit;
t_queue *estado_bloqueado;
*/

// PASAR A KERNEL. H PARA QUE LO UTILICE LARGO PLAZO

// MUTEX -
pthread_mutex_t *mutex_pcb_ejecutando;
pthread_mutex_t *mutex_estado_ejecutando;

// pthread_mutex_init(mutex, NULL) INICIALIZAR EN EL MAIN -ACORDARSE

// SEMAFOROS
sem_t *semaforo_ready;

void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado) // Añade un proceso a la cola New
{
    pthread_mutex_lock(cola_estado->mutex_estado);
    queue_push(cola_estado->estado, pcb);
    pthread_mutex_unlock(&mutex_estado_ejecutando);
}

// PASAR PROCESO DE READY A EXECUTE
void ready_a_execute(colaEstado *cola_ready)
{ // Esto sirve solo para FIFO y RR    - VER DE CAMBIAR LOGICA EN CASO DE VRR CON UN IF
    while (1)
    {
        pthread_mutex_lock(mutex_pcb_ejecutando); // Deberia ser un binario que marque cuando hay 1 procesaso en EXEC para que no haya 2 a la vez
        sem_wait(semaforo_ready);                 // Contador que resta uno, cuando un proceso pasa de ready a execute

        // if(FIFO or RR) =>

        t_pcb *pcb = obtener_siguiente_ready(cola_ready);
        // else (VRR)
        agregar_a_estado(pcb, cola_ready);

        // simular_ejecucion_proceso(pcb, "asd"); ----IMPLEMENTAR----

        log_info(logger, "Se agrego un proceso %d  a Execute desde Ready\n", pcb->cde->pid);
    }
}

// OBTENER SIGUIENTE PROCESO
t_pcb *obtener_siguiente_ready(colaEstado *cola_ready)
{
    pthread_mutex_lock(cola_ready->mutex_estado); // mutex para estado ready y controlar la condicion de carrera

    t_pcb *pcb_ready = queue_pop(cola_ready->estado); // obtiene y quita el proceso de la cola de ready || SECCION CRITICA

    pthread_mutex_unlock(cola_ready->mutex_estado);
    return pcb_ready;
}

/* void simular_ejecucion_proceso(t_pcb *proceso)
{
    int CE;

    t_cde *cde = obtener_cde(proceso);

    // enviar_paquete(paquete_a_exec, conexion_cpu);
    enviar_cde(conexion_cpu, cde, CE); // ENVIO PROCESO A PUERTO DISPATCH

    // semaforo de iniciar ejecucion
    while (proceso->quantum > 0)
    {
        if (proceso_completado())
        {
            break;
        }

        usleep(1000); // 1000 (1000MicroSegundos) ?? = 1 Quantum // Consultar con ayudante el tiempo
        proceso->quantum--;

        if (proceso->quantum == 0)
        {
            break;
        }
    }
}
 */
/* t_cde *obtener_cde(t_pcb *proceso)
{
    return proceso->cde;
} */

/* void enviar_cde(int conexion, t_cde *cde, int codOP) //----IMPLEMENTAR----
{
    t_paquete *paquete = crear_paquete_op_code(codOP);

    agregar_cde_a_paquete(paquete, cde);

    enviar_paquete(paquete, conexion);

    eliminar_paquete(paquete);
} */
// Agregamos el Contexto de Ejecucion a Paquete
/* void agregar_cde_a_paquete(t_paquete *paquete, t_cde *cde)
{
    // Agregamos PID
    // agregar_entero_a_paquete(paquete, cde->pid);
    log_trace(logger, "Se agrego el ID del proceso");

    // Agregamos INSTRUCCIONES
    // agregar_lista_instrucciones_a_paquete(paquete, cde->instrucciones);
    log_trace(logger, "Se agregaron las instrucciones del proceso");

    // Agregamos PC
    // agregar_entero_a_paquete(paquete, cde->pc);
    log_trace(logger, "Se agrego el program counter");

    // Agregamos REGISTROS
    //  agregar_registros_a_paquete(paquete, ce->registros_cpu);          //----IMPLEMENTAR----
    log_trace(logger, "Se agregaron los registros"); // crear la funcion para mandar los registros.

    // agregar_tabla_segmentos_a_paquete(paquete, ce->tabla_segmentos);  //----IMPLEMENTAR----
    //  log_trace(logger, "agrego tabla de segmentos");
} */

// serializaciones

/* void agregar_lista_instrucciones_a_paquete(t_paquete *paquete, t_list *instrucciones)
{

    int tamanio = list_size(instrucciones);

    agregar_entero_a_paquete(paquete, tamanio);

    for (int i = 0; i < tamanio; i++)
    {
        t_instruccion *instruccion = list_get(instrucciones, i);
        agregar_instruccion_a_paquete(paquete, instruccion);
    }
} */

/* void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion *instruccion)
{
    agregar_tipo_instruccion_a_paquete(paquete, instruccion->tipoInstruccion);
    agregar_entero_a_paquete(paquete, string_length(instruccion->argumento1));
    agregar_string_a_paquete(paquete, instruccion->argumento1);
    agregar_entero_a_paquete(paquete, string_length(instruccion->argumento1));
    agregar_string_a_paquete(paquete, instruccion->argumento2);
} */

/* void agregar_tipo_instruccion_a_paquete(t_paquete *paquete, t_tipoDeInstruccion tipo)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(t_tipoDeInstruccion));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &tipo, sizeof(t_tipoDeInstruccion));
    paquete->buffer->size += sizeof(t_tipoDeInstruccion);
} */

/* void agregar_entero_a_paquete(t_paquete *paquete, uint32_t x)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &x, sizeof(uint32_t));
    paquete->buffer->size += sizeof(uint32_t);
} */

/* void agregar_string_a_paquete(t_paquete *paquete, char *palabra)
{
    // agregar_entero_a_paquete(paquete,(int)strlen(palabra));
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(char *));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &palabra, sizeof(char *));
    paquete->buffer->size += (sizeof(char *));
} */
// AGREGADOS A COLAS DE ESTADOS CON SEMAFOROS

