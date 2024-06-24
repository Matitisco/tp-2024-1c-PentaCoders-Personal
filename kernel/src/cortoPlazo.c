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
    t_pcb *proceso = malloc(sizeof(t_pcb));
    while (1)
    {
        sem_wait(b_exec_libre);
        sem_wait(cola_ready_global->contador);
        proceso = transicion_ready_exec();
        proceso->estado = EXEC;
        log_info(logger, "Se agrego el proceso <%d> y PC <%d> a Execute desde Ready por FIFO\n", proceso->cde->pid, proceso->cde->PC);
        enviar_a_cpu_cde(proceso->cde);
    }
}

void planificar_por_rr()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));
    while (1)
    {
        sem_wait(b_exec_libre);
        sem_wait(contador_readys);
        proceso = transicion_ready_exec();
        proceso->estado = EXEC;
        log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por RR con Quantum: %d\n", proceso->cde->pid, QUANTUM);
        inicio_quantum();
        enviar_a_cpu_cde(proceso->cde);
    }
}

void planificar_por_vrr()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));
    while (1)
    {
        sem_wait(b_exec_libre);
        sem_wait(contador_readys);
        if (hayProcesosEnEstado(cola_ready_plus))
        {
            proceso = transicion_generica(cola_ready_plus, cola_exec_global, "corto");
            proceso->estado = EXEC;
            inicio_quantum(proceso->quantum);
            log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por VRR con Quantum: %d\n", proceso->cde->pid, proceso->quantum);
        }
        else
        {
            proceso = transicion_ready_exec();
            proceso->estado = EXEC;
            inicio_quantum(QUANTUM);
            log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por VRR con Quantum Normal: %d\n", proceso->cde->pid, QUANTUM);
        }
        enviar_a_cpu_cde(proceso->cde);
        timer = temporal_create();
    }
}

// QUANTUM

void inicio_quantum(int quantum)
{
    quantum_usable = quantum;
    log_info(logger, "Inicio de QUANTUM");
    pthread_create(&hiloQuantum, NULL, hilo_quantum, NULL);
    pthread_detach(&hiloQuantum);
    sem_post(sem_quantum);
}

void *hilo_quantum()
{
    while (1)
    {
        sem_wait(sem_quantum);
        sleep_ms(quantum_usable);
        enviar_op_code(socket_cpu_interrupt, PROCESO_INTERRUMPIDO_QUANTUM);
    }
}

// TRANSICIONES

// READY -> EXEC
t_pcb *transicion_ready_exec()
{

    t_pcb *proceso = sacar_procesos_cola(cola_ready_global);
    agregar_a_estado(proceso, cola_exec_global);
    return proceso;
}
// EXEC -> READY
void *transicion_exec_ready()
{
    while (1)
    {
        sem_wait(b_transicion_exec_ready);
        t_pcb *proceso = transicion_generica(cola_exec_global, cola_ready_global, "corto");
        proceso->cde = cde_interrumpido;
        proceso->estado = READY;

        sem_post(contador_readys);
        sem_post(b_exec_libre);

        log_info(logger, "Se desalojo el proceso %d - Motivo:", proceso->cde->pid);
    }
}
// EXEC -> BLOCKED
void *transicion_exec_blocked()
{
    while (1)
    {
        sem_wait(b_transicion_exec_blocked);
        sem_post(b_exec_libre);

        t_pcb *proceso = transicion_generica(cola_exec_global, cola_bloqueado_global, "corto");
        proceso->cde = cde_interrumpido;

        temporal_stop(timer);
        tiempo_transcurrido = temporal_gettime(timer);
        temporal_destroy(timer);

        log_info(logger, "Se bloqueo el proceso %d y PC %d", proceso->cde->pid, proceso->cde->PC);
        proceso->estado = BLOCKED;
    }
}
// BLOCKED -> READY
void *transicion_blocked_ready()
{
    while (1)
    {
        valorSemaforo(b_transicion_blocked_ready);
        sem_wait(b_transicion_blocked_ready);

        t_pcb *proceso;
        if (tiempo_transcurrido < QUANTUM)
        {

            proceso = transicion_generica(cola_bloqueado_global, cola_ready_plus, "corto"); // READY+
            proceso->estado = READY_PLUS;
            proceso->quantum = QUANTUM - tiempo_transcurrido;
            log_info(logger, "El proceso tiene un quantum restante de %d", proceso->quantum);
        }
        else
        {
            proceso = transicion_generica(cola_bloqueado_global, cola_ready_global, "corto"); // READY+
            proceso->estado = READY;
        }

        proceso->cde = cde_interrumpido;

        sem_post(contador_readys);

        log_info(logger, "Se desbloqueo el proceso %d y PC %d", proceso->cde->pid, proceso->cde->PC);
    }
}

// AUXILIARES

void signalInterruptor(int valor_interruptor, sem_t *interruptorSemaforo)
{
    int *valorSem = malloc(sizeof(int));
    sem_getvalue(interruptorSemaforo, valorSem);
    if (valor_interruptor == 0 && valorSem == -1)
    {
        valor_interruptor = 1;
        sem_post(interruptorSemaforo);
    }
}

void waitInterruptor(int valor_interruptor, sem_t *interruptorSemaforo)
{
    int *valorSem = malloc(sizeof(int));
    sem_getvalue(interruptorSemaforo, valorSem);
    if (valor_interruptor == 1 && valorSem == 0)
    {
        valor_interruptor = 0;
        sem_wait(interruptorSemaforo);
    }
}

void valorSemaforo(sem_t *semaforo)
{
    int *valor = malloc(sizeof(int));
    sem_getvalue(semaforo, valor);
    log_info(logger, "El valor del semaforo es: %d ", *valor);
}

int hayProcesosEnEstado(colaEstado *cola_estado)
{
    int *valor = malloc(sizeof(int));
    sem_getvalue(cola_estado->contador, valor);
    log_info(logger, "Hay %d procesos en el Estado %s", *valor, cola_estado->nombreEstado);
    if (*valor > 0)
        return 1;
    else
        return 0;
}

void enviar_a_cpu_cde(t_cde *cde)
{
    log_info(logger, "Proceso A Enviar: <%d>", cde->pid);
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
