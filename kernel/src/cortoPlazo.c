#include "../include/cortoPlazo.h"

pthread_mutex_t *mutex_pcb_ejecutando;
pthread_mutex_t *mutex_estado_ejecutando;
pthread_mutex_t *mutex_cola_ready;
pthread_mutex_t *mutex_cola_exec;
//t_temporal *quantum;
t_temporal *timer;
int tiempo_transcurrido;
char* plani = "corto";

int quantum_usable;








// CORTO PLAZO
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

void planificar_por_fifo()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));
    while (1)
    {

        sem_wait(b_exec_libre);
        

        proceso = transicion_ready_exec();

        proceso->estado = EXEC;

        enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO);
        enviar_cde(socket_cpu_dispatch, proceso->cde);
        log_info(logger, "Se agrego el proceso <%d> y PC <%d> a Execute desde Ready por FIFO\n", proceso->cde->pid, proceso->cde->PC);
    }
}

// ACA ARRANCAN CAMBIOS HECHOS POR MATI

void planificar_por_rr()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));

    while (1)
    {
       

        sem_wait(b_exec_libre);


        log_info(logger, "Proceso a enviar: %d", proceso->cde->pid);
        log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por ROUND ROBIN con quantum: %d\n", proceso->cde->pid, QUANTUM);


        inicio_quantum();
        
        enviar_a_cpu_cde(proceso->cde);
        /* enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO); // PASA A ESTADO EXEC
        enviar_cde(socket_cpu_dispatch, proceso->cde); */
    }
}






void planificar_por_vrr()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));

    while (1)
    {
        sem_wait(b_exec_libre); // deja de estar libre exec

        bloquearSiReadysVacios();

        if(hayProcesosEnEstado(cola_ready_plus))
        {
            proceso = transicion_generica(cola_ready_plus,cola_exec_global,"corto");
            inicio_quantum(proceso->quantum);
            log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por VIRTUAL ROUND ROBIN con quantum: %d\n", proceso->cde->pid, proceso->quantum);
        }
        else
        {
            proceso = transicion_ready_exec();
            inicio_quantum(QUANTUM);
            log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por VIRTUAL ROUND ROBIN con quantum NORMAL: %d\n", proceso->cde->pid, QUANTUM);
        }
        
        enviar_a_cpu_cde(proceso->cde);

        

        // Iniciar temporal
        timer = temporal_create();
    
        
    }
}


void bloquearSiReadysVacios()
{
    if(hayProcesosEnEstado(cola_ready_global) == 0 && hayProcesosEnEstado(cola_ready_plus) == 0) 
    {
        sem_wait(b_switch_readys);  //Si hay procesos en la cola avanza
    }
}

void desbloquearSiReadysVacios(){   //SIEMPRE PONER ANTES DE LA TRANSICION
    if(hayProcesosEnEstado(cola_ready_global) == 0 && hayProcesosEnEstado(cola_ready_plus) == 0)
    {
        sem_post(b_switch_readys);
    }
}


void signalInterruptor(int valor_interruptor, sem_t *interruptorSemaforo)
{
    int *valorSem = malloc(sizeof(int));
    sem_getvalue(interruptorSemaforo, valorSem);
    if(valor_interruptor == 0 &&  valorSem == -1)
    {
        valor_interruptor = 1;
        sem_post(interruptorSemaforo);
    }
}

void waitInterruptor(int valor_interruptor, sem_t *interruptorSemaforo)
{
    int *valorSem = malloc(sizeof(int));
    sem_getvalue(interruptorSemaforo, valorSem);
    if(valor_interruptor == 1 && valorSem == 0 )
    {
        valor_interruptor = 0;
        sem_wait(interruptorSemaforo);
    }
}



int hayProcesosEnEstado(colaEstado* cola_estado)
{
    int *valor = malloc(sizeof(int));
    sem_getvalue(cola_estado->contador, valor);
    log_info(logger, "Hay %d procesos en el Estado %s",*valor, cola_estado->nombreEstado);
    if(*valor > 0)
        return 1;
    else
        return 0;
}


void enviar_cde(int conexion, t_cde *cde)
{
    tipo_buffer *buffer = crear_buffer();
    agregar_cde_buffer(buffer, cde);
    enviar_buffer(buffer, socket_cpu_dispatch);
    destruir_buffer(buffer);
}

void enviar_a_cpu_cde(t_cde *cde)
{
    log_info(logger, "Proceso a enviar: %d", cde->pid);
    enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO);
    enviar_cde(socket_cpu_dispatch, cde);
}

t_pcb *transicion_ready_exec()
{
    t_pcb *proceso = transicion_generica(cola_ready_global,cola_exec_global,"corto");
    proceso->estado = EXEC;
    
    log_info(logger, "PROCESO SACADO DE READY: %d", proceso->cde->pid);

    return proceso;
}

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
        enviar_cod_enum(socket_cpu_interrupt, PROCESO_INTERRUMPIDO_QUANTUM);
    }
}

void *transicion_exec_ready()
{ // Falta ver el tema del motivo para que sea generico segun el caso
    while (1)
    {
        sem_wait(b_transicion_exec_ready);
       

        /* 
        t_pcb *proceso = sacar_procesos_cola(cola_exec_global, plani);
        agregar_a_estado(proceso, cola_ready_global); */
        desbloquearSiReadysVacios();
        t_pcb* proceso = transicion_generica(cola_exec_global,cola_ready_global,"corto");
        proceso->cde = cde_interrumpido;
        proceso->estado = READY;    // es un puntero y puedo cambiar sus cosas desde aca
        
        sem_post(b_exec_libre);


        log_info(logger, "Se desalojo el proceso %d - Motivo:", proceso->cde->pid);
    }
}

void *transicion_exec_blocked() // mover a largo plazo
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

void *transicion_blocked_ready() // mover a largo plazo
{
    while (1)
    {
        sem_wait(b_transicion_blocked_ready);
        t_pcb *proceso;
        desbloquearSiReadysVacios();
        if( tiempo_transcurrido < QUANTUM ){    //&& tiempo_transcurrido > 500 ??

            proceso = transicion_generica(cola_bloqueado_global,cola_ready_plus, "corto"); //READY+
            proceso->estado = READY_PLUS;
            proceso->quantum = QUANTUM - tiempo_transcurrido;
            log_info(logger, "El proceso tiene un quantum restante de %d", proceso->quantum);
        }
        else{
            
            proceso = transicion_generica(cola_bloqueado_global,cola_ready_global, "corto"); //READY+
            proceso->estado = READY;
        }
        
        
        proceso->cde = cde_interrumpido;

        log_info(logger, "Se desbloqueo el proceso %d y PC %d", proceso->cde->pid, proceso->cde->PC);
        
    }
}

/* void *vrr_transicion_readyplus_exec()
{
    sem_wait(b_transicion_readyplus_exec);
    t_pcb *proceso = transicion_generica(cola_ready_plus,cola_exec_global,"corto");
    proceso->estado = EXEC;
    
    log_info(logger, "PROCESO SACADO DE READY: %d", proceso->cde->pid);

    return proceso;

} */


