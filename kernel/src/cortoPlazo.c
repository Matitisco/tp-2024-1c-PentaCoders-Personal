#include "../include/cortoPlazo.h"

/* Los procesos que estén en estado READY serán planificados mediante uno de los siguientes algoritmos:*/

// PASAR A KERNEL. H PARA QUE LO UTILICE LARGO PLAZO

pthread_mutex_t *mutex_pcb_ejecutando;
pthread_mutex_t *mutex_estado_ejecutando;
pthread_mutex_t *mutex_cola_ready;
pthread_mutex_t *mutex_cola_exec;
t_temporal *quantum;

// CORTO PLAZO
void *corto_plazo()
{
    // op_code estado_planificacion = INICIAR_PLANIFICACION;
    // log_info(logger, "--------------Planificador de Corto Plazo Iniciado-------------- \n");
    iniciar_sem_cp();

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
        quantum = temporal_create();
        planificar_por_vrr();
    }
    else
    {
        return (void *)1;
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
        sem_wait(b_reanudar_corto_plazo);

        cambiar_procesoActual_readyARunning();

        /*         if (hayInstruccionBloqueante()) // si es true
                {
                    // log_info(logger, "Se quito el proceso %d  de Execute \n", proceso->cde->pid);
                    cambiar_procesoActual_readyARunning();
                } */
    }
}

void cambiar_procesoActual_readyARunning()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));
    sem_wait(b_exec_libre);
    sem_wait(cola_ready_global->contador); // cantidad procesos en ready

    proceso = sacar_procesos_cola(cola_ready_global); // SALE DE READY
    agregar_a_estado(proceso, cola_exec_global);

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
    agregar_cde_buffer(buffer, cde);
    enviar_buffer(buffer, socket_cpu_dispatch);
    destruir_buffer(buffer);
}
// ROUND ROBIN

/* t_pcb *proceso = malloc(sizeof(t_pcb));
    sem_wait(b_exec_libre); // deja de estar libre exec  EL SEM_POST DEBE ESTAR EN CPU, YA QUE ES CUANDO DEJAMOS LA COLA DE READY LIBRE.
    sem_wait(cola_ready_global->contador); // cantidad procesos en ready

    proceso = sacar_procesos_cola(cola_ready_global); // SALE DE READY
    agregar_a_estado(proceso, cola_exec_global);


    // ENVIA A CPU
    enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO); // PASA A ESTADO EXEC
    enviar_cde(socket_cpu_dispatch, proceso->cde);
    log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por FIFO\n", proceso->cde->pid);

 */
void planificar_por_rr()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));

    while (1)
    {
        sem_wait(b_exec_libre);                // deja de estar libre exec
        sem_wait(cola_ready_global->contador); // contador de procesos en ready

        proceso = sacar_procesos_cola(cola_ready_global); // SALE DE READY
        agregar_a_estado(proceso, cola_exec_global);
        log_info(logger, "Proceso a enviar: %d", proceso->cde->pid);
        log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por ROUND ROBIN con quantum: %d\n", proceso->cde->pid, QUANTUM);

        log_info(logger, "Inicio de QUANTUM");
        pthread_create(&hiloQuantum, NULL, hilo_quantum, NULL);
        sem_post(sem_quantum);
        enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO); // PASA A ESTADO EXEC
        enviar_cde(socket_cpu_dispatch, proceso->cde);
    }
}

void *hilo_quantum()
{
    while (1)
    {
        sem_wait(sem_quantum);
        usleep(QUANTUM);
        // if (!llego_proceso())
        //{
        enviar_cod_enum(socket_cpu_interrupt, PROCESO_INTERRUMPIDO_QUANTUM);
        //}
        // else
        //{
        // log_info(logger, "El proceso salio antes del fin de quantum");
        //}
    }
}

void *transicion_exec_ready()
{ // Falta ver el tema del motivo para que sea generico segun el caso

    while (1)
    {
        sem_wait(b_transicion_exec_ready);
        t_pcb *proceso = sacar_procesos_cola(cola_exec_global);
        proceso->cde = cde_interrumpido_por_dispatch;
        agregar_a_estado(proceso, cola_ready_global); // moverlo a la cola de exit, hay un lugar en memoria
        sem_post(b_exec_libre);
        log_info(logger, "Se desalojo el proceso %d - Motivo:", proceso->cde->pid);
        // liberar_proceso(proceso);
    }
}

void replanificar_por_rr(t_pcb *proceso)
{

    // enviar_cod_enum(socket_cpu_interrupt, PROCESO_INTERRUMPIDO);
    // enviar_cde(socket_cpu_dispatch, proceso->cde); // Envio que se pare el proceso a la conexion de interrupt

    // Hay que hacer que un hilo reciba el retorno de la interrupcion de CPU y que habilite con un binario el hilo exec=>ready

    /* proceso = sacar_procesos_cola(cola_ready_global); // lo saco de running
    agregar_a_estado(proceso, cola_exec_global); // lo meto en ready */

    /* t_pcb *otro_proceso = malloc(sizeof(t_pcb));

    otro_proceso = sacar_procesos_cola(cola_ready_global); // SALE DE READY
    agregar_a_estado(otro_proceso, cola_exec_global); */
}
// VIRTUAL ROUND ROBIN
void planificar_por_vrr() {}