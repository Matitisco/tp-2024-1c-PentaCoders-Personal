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

// pthread_mutex_init(mutex, NULL) INICIALIZAR EN EL MAIN -ACORDARSE
/* sem_t *semaforo_ready;
sem_t *procesos_en_ready;
sem_t *procesos_en_exec; */
/* sem_t *exec_libre; */

// CORTO PLAZO
void *corto_plazo()
{
    // op_code estado_planificacion = INICIAR_PLANIFICACION;
    log_info(logger, "--------------Planificador de Corto Plazo Iniciado-------------- \n");

    if (strcmp(valores_config->algoritmo_planificacion, "FIFO") == 0)
    {
        planificar_por_fifo();
    }
    else if (strcmp(valores_config->algoritmo_planificacion, "RR") == 0)
    {
        planificar_por_rr();
    }
    else if (strcmp(valores_config->algoritmo_planificacion, "VRR") == 0)
    {
        planificar_por_vrr();
    }
}
// FIFO
void planificar_por_fifo()
{
    // el primero que llega es el primero que ejecuto

    /*
    Una vez seleccionado el siguiente proceso a ejecutar,
    se lo transicionará al estado EXEC y se enviará su Contexto de
    Ejecución al CPU a través del puerto de dispatch, quedando a la
    espera de recibir dicho contexto actualizado después de la ejecución,
    junto con un motivo de desalojo por el cual fue desplazado a manejar.
    En caso que el algoritmo requiera desalojar al proceso en ejecución, se
    enviará una interrupción a través de la conexión de interrupt para forzar el desalojo del mismo.
    Al recibir el Contexto de Ejecución del proceso en ejecución, en caso de
    que el motivo de desalojo implique replanificar se seleccionará el siguiente
    proceso a ejecutar según indique el algoritmo. Durante este período la CPU se quedará esperando el nuevo contexto.*/
    t_pcb *proceso = malloc(sizeof(t_pcb));
    while (1)
    {
        sem_wait(exec_libre); // deja de estar libre exec
        sem_wait(procesos_en_ready);
        proceso = sacar_procesos_cola(cola_ready_global, procesos_en_ready); // SALE DE READY
        agregar_a_estado(proceso, cola_exec_global, procesos_en_exec);
        // ENVIA A CPU
        enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO); // PASA A ESTADO EXEC

        enviar_cde(socket_cpu_dispatch, proceso->cde);

        log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por FIFO\n", proceso->cde->pid);
    }
}

void enviar_cde(int conexion, t_cde *cde)
{
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, cde->registro->PC);  // sacar path y sacar lista_instrucciones de CDE
    agregar_buffer_para_enterosUint32(buffer, cde->pid); // agrego al pid el buffer
    agregar_buffer_para_registros(buffer, *(cde->registro));
    enviar_buffer(buffer, socket_cpu_dispatch);
}

// ROUND ROBIN
void planificar_por_rr()
{
}
// VIRTUAL ROUND ROBIN
void planificar_por_vrr() {}

// PASAR PROCESO DE READY A EXECUTE
/* void ready_a_execute(colaEstado *cola_ready)
{ // Esto sirve solo para FIFO y RR    - VER DE CAMBIAR LOGICA EN CASO DE VRR CON UN IF
    while (1)
    {
        pthread_mutex_lock(mutex_pcb_ejecutando); // Deberia ser un binario que marque cuando hay 1 procesaso en EXEC para que no haya 2 a la vez
        sem_wait(semaforo_ready);                 // Contador que resta uno, cuando un proceso pasa de ready a execute

        // if(FIFO or RR) =>

        t_pcb *pcb = obtener_siguiente_ready(cola_ready);
        // else (VRR)
        agregar_a_estado(pcb, cola_ready, procesos_en_ready);

        // simular_ejecucion_proceso(pcb, "asd"); ----IMPLEMENTAR----

        log_info(logger, "Se agrego un proceso %d  a Execute desde Ready\n", pcb->cde->pid);
    }
}







/*
void simular_ejecucion_proceso(t_pcb *proceso)
{
    // int CE;

    // t_cde *cde = obtener_cde(proceso);

    // enviar_paquete(paquete_a_exec, conexion_cpu);
    // enviar_cde(conexion_cpu, cde, CE); // ENVIO PROCESO A PUERTO DISPATCH

    // semaforo de iniciar ejecucion
    while (QUANTUM > 0)
    {
        if (proceso_completado())
        {
            break;
        }

        usleep(1000); // 1000 (1000MicroSegundos) ?? = 1 Quantum // Consultar con ayudante el tiempo
        QUANTUM--;

        if (QUANTUM == 0)
        {
            break;
        }
    }
}
*/
