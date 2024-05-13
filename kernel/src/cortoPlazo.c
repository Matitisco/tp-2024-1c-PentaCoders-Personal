#include "../include/cortoPlazo.h"
/*

EXEC -> READY // Se desata cuando CPU devuelve el CTE y el motivoDeDesalojo (por Interrupt?)


READY -> EXEC // No se

EXEC -> BLOCK //Cuando se completo la rafaga en CPU y se recibe CTE y mDD (por Dispatch)
BLOCK -> READY // Cuando cumple su operacion en IO

BLOCK -> EXIT // Cuando su operacion IO era lo ultimo pendiente
*/

/* Los procesos que estén en estado READY serán planificados mediante uno de los siguientes algoritmos:
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
el siguiente proceso a ejecutar según indique el algoritmo. Durante este período la CPU se quedará esperando el nuevo contexto. */

// PASAR A KERNEL. H PARA QUE LO UTILICE LARGO PLAZO

pthread_mutex_t *mutex_pcb_ejecutando;
pthread_mutex_t *mutex_estado_ejecutando;
pthread_mutex_t *mutex_cola_ready;
pthread_mutex_t *mutex_cola_exec;
t_temporal *quantum;
// pthread_mutex_init(mutex, NULL) INICIALIZAR EN EL MAIN -ACORDARSE
/* sem_t *semaforo_ready;
sem_t *procesos_en_ready;
sem_t *procesos_en_exec; */
/* sem_t *exec_libre; */
//
// CORTO PLAZO
void *corto_plazo()
{
    // op_code estado_planificacion = INICIAR_PLANIFICACION;
    log_info(logger, "--------------Planificador de Corto Plazo Iniciado-------------- \n");
    iniciar_sem_cp();
    if (strcmp(valores_config->algoritmo_planificacion, "FIFO") == 0)
    {
        planificar_por_fifo();
    }
    else if (strcmp(valores_config->algoritmo_planificacion, "RR") == 0)
    {
        quantum = temporal_create();
        planificar_por_rr();
    }
    else if (strcmp(valores_config->algoritmo_planificacion, "VRR") == 0)
    {
        quantum = temporal_create();
        planificar_por_vrr();
    }
}

void iniciar_sem_cp()
{
    mutex_pcb_ejecutando = malloc(sizeof(pthread_mutex_t));
    mutex_estado_ejecutando = malloc(sizeof(pthread_mutex_t));
    mutex_cola_ready = malloc(sizeof(pthread_mutex_t));
    mutex_cola_exec = malloc(sizeof(pthread_mutex_t));

    pthread_mutex_init(mutex_pcb_ejecutando, NULL);
    pthread_mutex_init(mutex_estado_ejecutando, NULL);
    pthread_mutex_init(mutex_cola_ready, NULL);
    pthread_mutex_init(mutex_cola_exec, NULL);
}

// FIFO
void planificar_por_fifo()
{
    while (1)
    {
        cambiar_procesoActual_readyARunning();

        if (hayInstruccionBloqueante()) // si es true
        {
            // log_info(logger, "Se quito el proceso %d  de Execute \n", proceso->cde->pid);
            cambiar_procesoActual_readyARunning();
        }
    }
}

void cambiar_procesoActual_readyARunning()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));
    sem_wait(exec_libre); // deja de estar libre exec  EL SEM_POST DEBE ESTAR EN CPU, YA QUE ES CUANDO DEJAMOS LA COLA DE READY LIBRE.
    sem_wait(procesos_en_ready);
    pthread_mutex_lock(mutex_cola_ready);                                // cantidad procesos en ready
    proceso = sacar_procesos_cola(cola_ready_global, procesos_en_ready); // SALE DE READY
    pthread_mutex_unlock(mutex_cola_ready);

    pthread_mutex_lock(mutex_cola_exec);
    agregar_a_estado(proceso, cola_exec_global, procesos_en_exec);
    pthread_mutex_unlock(mutex_cola_exec);

    // ENVIA A CPU
    enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO); // PASA A ESTADO EXEC
    enviar_cde(socket_cpu_dispatch, proceso->cde);
    log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por FIFO\n", proceso->cde->pid);
}

int hayInstruccionBloqueante()
{
    // recibamos cod enum de cpi
    op_code operacion = recibir_operacion(socket_cpu_interrupt); // recibimos dle socket interrupt por que es bloqueante
    if (operacion == PROCESO_INTERRUMPIDO)
    {
        // Agregar en cpu una funcio que diga si una isntruccion es bloqueantes o no

        return 1;
    }
    else
    {
        return 0;
    }
}

void enviar_cde(int conexion, t_cde *cde)
{
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, cde->registro->PC); // sacar path y sacar lista_instrucciones de CDE
    agregar_buffer_para_enterosUint32(buffer, cde->pid);          // agrego al pid el buffer
    agregar_buffer_para_registros(buffer, *(cde->registro));
    enviar_buffer(buffer, socket_cpu_dispatch);
}

// ROUND ROBIN
void planificar_por_rr()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));

    // primer proceso que meto

    pthread_mutex_lock(mutex_cola_ready);
    proceso = sacar_procesos_cola(cola_ready_global, procesos_en_ready); // SALE DE READY
    pthread_mutex_unlock(mutex_cola_ready);

    pthread_mutex_lock(mutex_cola_exec);
    agregar_a_estado(proceso, cola_exec_global, procesos_en_exec);
    pthread_mutex_unlock(mutex_cola_exec);
    enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO); // PASA A ESTADO EXEC
    enviar_cde(socket_cpu_dispatch, proceso->cde);
    log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por ROUND ROBIN con quantum: %d\n", proceso->cde->pid, QUANTUM);
    while (1)
    {
        // P1 4 RAFAGAS q = 2
        while (temporal_gettime(quantum) != QUANTUM)
        {
            // simular la ejecucion de un proceso
            // sleep() // Cuando el cronometros llego al quatum que queremos <-ojo que sleep es bloqueante
        }
        temporal_stop(quantum);
        replanificar_por_rr(proceso); // se para el timer
        temporal_destroy(quantum);
        // caundo las rafagas de cpu son menores a las definidas por el quantum, la cpu nos debe avisar que el proceso se bloqueo o termino.
    }
    // ENVIA A CPU
}
void replanificar_por_rr(t_pcb *proceso)
{

    enviar_cod_enum(socket_cpu_interrupt, DETENER_PROCESO_FIN_DE_QUANTUM);
    enviar_cde(socket_cpu_dispatch, proceso->cde); // Envio que se pare el proceso a la conexion de interrupt
    pthread_mutex_lock(mutex_cola_ready);
    proceso = sacar_procesos_cola(cola_ready_global, procesos_en_exec); // lo saco de running
    pthread_mutex_unlock(mutex_cola_ready);

    pthread_mutex_lock(mutex_cola_exec);
    agregar_a_estado(proceso, cola_exec_global, procesos_en_ready); // lo meto en ready
    pthread_mutex_unlock(mutex_cola_exec);

    t_pcb *otro_proceso = malloc(sizeof(t_pcb));

    pthread_mutex_lock(mutex_cola_ready);
    otro_proceso = sacar_procesos_cola(cola_ready_global, procesos_en_ready); // SALE DE READY
    pthread_mutex_unlock(mutex_cola_ready);

    pthread_mutex_lock(mutex_cola_exec);
    agregar_a_estado(otro_proceso, cola_exec_global, procesos_en_exec);
    pthread_mutex_unlock(mutex_cola_exec);
}
// VIRTUAL ROUND ROBIN
void planificar_por_vrr() {}