#include "../include/cortoPlazo.h"

/* Los procesos que estén en estado READY serán planificados mediante uno de los siguientes algoritmos:*/

// PASAR A KERNEL. H PARA QUE LO UTILICE LARGO PLAZO

pthread_mutex_t *mutex_pcb_ejecutando;
pthread_mutex_t *mutex_estado_ejecutando;
pthread_mutex_t *mutex_cola_ready;
pthread_mutex_t *mutex_cola_exec;
t_temporal *quantum;

t_temporal timer;
int tiempo_transcurrido;

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

void planificar_por_fifo()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));
    while (1)
    {
        //sem_wait(b_reanudar_corto_plazo);
        
        sem_wait(b_exec_libre);
        
        proceso = transicion_ready_exec();

        enviar_a_cpu_cde(proceso->cde);

        log_info(logger, "Se agrego el proceso %d y PC %d a Execute desde Ready por FIFO\n", proceso->cde->pid, proceso->cde->PC);
    }
}





void planificar_por_rr()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));

    while (1)
    {
        //sem_wait(b_reanudar_corto_plazo);
        sem_wait(b_exec_libre);                // deja de estar libre exec

        proceso = transicion_ready_exec();

        log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por ROUND ROBIN con quantum: %d\n", proceso->cde->pid, QUANTUM);

        inicio_quantum();

        enviar_a_cpu_cde(proceso->cde);
        
    }
}


void planificar_por_vrr() 
{
    t_pcb *proceso = malloc(sizeof(t_pcb));

    while (1)
    {
        sem_wait(b_exec_libre);                // deja de estar libre exec

        proceso = transicion_ready_exec();

        log_info(logger, "Se agrego el proceso %d  a Execute desde Ready por ROUND ROBIN con quantum: %d\n", proceso->cde->pid, QUANTUM);

        inicio_quantum();

        enviar_a_cpu_cde(proceso->cde);

        //Iniciar temporal
        timer = temporal_create();

        t_pcb *proceso = recibir_proceso_de_cpu(); //VER COMO GESTIONAMOS EL CAMBIO DE ESTADO, DEBERIA MANDARLO A BLOCKED PERO GUARDAR EL QUANTUM


        //Detener temporal
        temporal_stop(timer);
        tiempo_transcurrido = temporal_gettime(timer);
        temporal_destroy(timer);
        
        
        
    }
}

void salida_block(){
    //Cuando llega de block

    
}

void *transicion_blocked_ready() // BLOCK => READY | READY+, ver como implementar
{
    while (1)
    {
        sem_wait(b_transicion_blocked_ready);
        t_pcb *proceso = sacar_procesos_cola(cola_bloqueado_global);

        if( tiempo_transcurrido < QUANTUM){
            proceso->quantum = QUANTUM - tiempo_transcurrido;
            //agregar_a_estado(proceso, cola_ready_extra_global);    //READY+
            //proceso->estado = READY_EXTRA;
        }
        else{
            agregar_a_estado(proceso,cola_ready_global);    //READY
            proceso->estado = READY;
        }


        log_info(logger, "Se desbloqueo el proceso %d y PC %d", proceso->cde->pid, proceso->cde->PC);
        
        
    }
}


void enviar_cde(int conexion, t_cde *cde)
{
    tipo_buffer *buffer = crear_buffer();
    agregar_cde_buffer(buffer, cde);
    enviar_buffer(buffer, socket_cpu_dispatch);
    destruir_buffer(buffer);
}

void enviar_a_cpu_cde(t_cde* cde){
    log_info(logger, "Proceso a enviar: %d", cde->pid);
    enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO); // PASA A ESTADO EXEC
    enviar_cde(socket_cpu_dispatch, cde);
}

t_pcb *transicion_ready_exec(){
    t_pcb *proceso = sacar_procesos_cola(cola_ready_global); // SALE DE READY
    agregar_a_estado(proceso, cola_exec_global);
    proceso->estado = EXEC;
    
    return proceso;
}

void inicio_quantum(){
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
        proceso->cde = cde_interrumpido;
        proceso->estado = READY;
        agregar_a_estado(proceso, cola_ready_global); // moverlo a la cola de exit, hay un lugar en memoria
        sem_post(b_exec_libre);
        
        //sem_post(b_reanudar_corto_plazo);
        log_info(logger, "Se desalojo el proceso %d - Motivo:", proceso->cde->pid);
        // liberar_proceso(proceso);
        //free(proceso);  NO HAY QUE LIBERAR MEMORIA PORQUE MATAMOS EL BLOQUE DE MEMORIA DONDE ESTA EL PROCESO Y NO PODEMOS ACCEDER MAS A EL
    }
}

void *transicion_exec_blocked() // mover a largo plazo
{
    while (1)
    {
        sem_wait(b_transicion_exec_blocked);
        sem_post(b_exec_libre);
        t_pcb *proceso = sacar_procesos_cola(cola_exec_global);
        proceso->cde = cde_interrumpido;
        agregar_a_estado(proceso, cola_bloqueado_global);
        log_info(logger, "Se bloqueo el proceso %d y PC %d", proceso->cde->pid, proceso->cde->PC);
        proceso->estado = BLOCKED;
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
