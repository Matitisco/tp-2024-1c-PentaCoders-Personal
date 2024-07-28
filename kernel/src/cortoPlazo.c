#include "../include/cortoPlazo.h"

pthread_mutex_t *mutex_pcb_ejecutando;
pthread_mutex_t *mutex_estado_ejecutando;
pthread_mutex_t *mutex_cola_ready;
pthread_mutex_t *mutex_cola_exec;
t_temporal *timer;
char *plani = "corto";
int tiempo_transcurrido;
int quantum_usable;

void *corto_plazo()
{
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
        planificar_por_vrr();
    }
    return (void *)1;
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

// ALGORITMOS DE PLANIFICACION

void planificar_por_fifo()
{
    while (1)
    {
        sem_wait(b_exec_libre);
        t_pcb *proceso = transicion_ready_exec();
        log_info(logger, "PID: <%d> - Estado Anterior: <READY+> - Estado Actual: <EXECUTE>", proceso->cde->pid);
        enviar_a_cpu_cde(proceso->cde);

        

    }
}

void planificar_por_rr()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));
    while (1)
    {
        sem_wait(b_exec_libre);
        proceso = transicion_ready_exec();
        log_info(logger, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXECUTE>", proceso->cde->pid);
        proceso->estado = EXEC;
        enviar_a_cpu_cde(proceso->cde);
    
        inicio_quantum(QUANTUM);
    }
}

void planificar_por_vrr()
{
    t_pcb *proceso;
    while (1)
    {
        sem_wait(b_exec_libre);

        sem_wait(contador_readys);

        if (hayProcesosEnEstado(cola_ready_plus))
        {
            proceso = transicion_generica(cola_ready_plus, cola_exec_global, "corto");
            log_info(logger, "PID: <%d> - Estado Anterior: <READY+> - Estado Actual: <EXECUTE>", proceso->cde->pid);
            proceso->estado = EXEC;
            enviar_a_cpu_cde(proceso->cde);
           
            inicio_quantum(proceso->quantum);
        }
        else
        {
            proceso = transicion_ready_exec();
            log_info(logger, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXECUTE>", proceso->cde->pid);
            proceso->estado = EXEC;
            enviar_a_cpu_cde(proceso->cde);
            inicio_quantum(QUANTUM);
        }
        if (timer != NULL)
        {
            temporal_destroy(timer);
        }
        timer = temporal_create();
    }
}

// QUANTUM

void inicio_quantum(int quantum)
{
    quantum_usable = quantum;
    pthread_create(&hiloQuantum, NULL, hilo_quantum, NULL);
    pthread_detach(&hiloQuantum);
}

void *hilo_quantum()
{
    sleep_ms(quantum_usable);
    enviar_op_code(socket_cpu_interrupt, PROCESO_INTERRUMPIDO_QUANTUM);
}
// TRANSICIONES

// READY -> EXEC
t_pcb *transicion_ready_exec()
{
    t_pcb *proceso = transicion_generica(cola_ready_global, cola_exec_global, "corto");
    proceso->estado = EXEC;
    return proceso;
}
// EXEC -> READY
void *transicion_exec_ready()
{
    while (1)
    {
        sem_wait(b_transicion_exec_ready);
        t_pcb *proceso = transicion_generica(cola_exec_global, cola_ready_global, "exec_ready");
        free(proceso->cde->registros);
        free(proceso->cde);
        proceso->cde = cde_interrumpido;
        proceso->estado = READY;

    
        sem_post(contador_readys);
        sem_post(b_exec_libre);

         //log obligatorio
        log_info(logger, "PID: <%d> - Estado Anterior: <EXECUTE> - Estado Actual: <READY>", proceso->cde->pid);
    }
}
// EXEC -> BLOCKED
void *transicion_exec_blocked()
{
    while (1)
    {
        sem_wait(b_transicion_exec_blocked);
        sem_post(b_exec_libre);

        t_pcb *proceso = transicion_generica(cola_exec_global, cola_bloqueado_global, "exec_blocked");
        proceso->cde = cde_interrumpido;

        if (strcmp(valores_config->algoritmo_planificacion, "VRR") == 0)
        {
            temporal_stop(timer);
            tiempo_transcurrido = temporal_gettime(timer);
            // temporal_destroy(timer);
        }
        proceso->estado = BLOCKED;
        log_info(logger, "PID: <%d> - Estado Anterior: <EXECUTE> - Estado Actual: <BLOCKED>", proceso->cde->pid);
    }
}
// BLOCKED -> READY
void *transicion_blocked_ready()
{
    while (1)
    {
        sem_wait(b_transicion_blocked_ready);

        t_pcb *proceso;
        if (strcmp(valores_config->algoritmo_planificacion, "VRR") == 0)
        {
            if (tiempo_transcurrido < QUANTUM)
            {

                proceso = transicion_generica(cola_bloqueado_global, cola_ready_plus, "blocked_ready"); // READY+
                proceso->estado = READY_PLUS;
                proceso->quantum = QUANTUM - tiempo_transcurrido;
            }
            else
            {
                proceso = transicion_generica(cola_bloqueado_global, cola_ready_global, "blocked_ready"); // READY+
                proceso->estado = READY;
            }
        }
        else
        {
            proceso = transicion_generica(cola_bloqueado_global, cola_ready_global, "corto");
            proceso->estado = READY;
        }

        sem_post(contador_readys);

        log_info(logger, "PID: <%d> - Estado Anterior: <BLOCKED> - Estado Actual: <READY>", proceso->cde->pid);
    }
}

// AUXILIARES

int valorSemaforo(sem_t *semaforo)
{
    int *valor = malloc(sizeof(int));
    sem_getvalue(semaforo, valor);
    log_info(logger, "El valor del semaforo es: %d ", *valor);
    return *valor;
}

int hayProcesosEnEstado(colaEstado *cola_estado)
{
    int *valor = malloc(sizeof(int));
    sem_getvalue(cola_estado->contador, valor);
    log_info(logger, "Hay %d procesos en el Estado %s", *valor, cola_estado->nombreEstado);
    if (*valor > 0)
    {
        free(valor);
        return 1;
    }
    else
    {
        free(valor);
        return 0;
    }
}

void enviar_a_cpu_cde(t_cde *cde)
{
    enviar_op_code(socket_cpu_dispatch, EJECUTAR_PROCESO);
    enviar_cde(socket_cpu_dispatch, cde);
}

void enviar_cde(int conexion, t_cde *cde)
{
    tipo_buffer *buffer = crear_buffer();
    agregar_cde_buffer(buffer, cde);
    enviar_buffer(buffer, socket_cpu_dispatch);
    destruir_buffer(buffer);
}
