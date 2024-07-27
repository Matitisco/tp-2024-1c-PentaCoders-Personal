#include "../include/largoPlazo.h"

int pid_a_sacar;

void *largo_plazo()
{
    while (1)
    {
        sem_wait(cant_procesos_en_new);
        // sem_wait(binario_menu_lp); SI SE DEJA ESTO SE TRABA A VECES
        sem_wait(GRADO_MULTIPROGRAMACION);
        //sem_wait(b_reanudar_largo_plazo);
        t_pcb *proceso = transicion_new_a_ready();
        log_info(logger, "PID: %d - Estado Anterior: <NEW> - Estado Actual: <READY>", proceso->cde->pid);
        //char* lista = lista_pid();
        log_info(logger, "Cola Ready / Ready Prioridad : [%s]", lista_pid());
        //free(lista);
        /* if (habilitar_planificadores == 1)
        {
            //sem_post(b_reanudar_largo_plazo);
        } */
    }
}

char *lista_pid()
{
    char *pids_en_ready = malloc(1024);
    for (int i = 0; i < list_size(cola_ready_global->estado); i++)
    {
        t_pcb *pcb = list_get(cola_ready_global->estado, i);
        if (pcb->cde->pid != 0)
        {
            char *pid = string_itoa(pcb->cde->pid);
            strcat(pids_en_ready, pid);
        }
        else
        {
            strcat(pids_en_ready, "0");
            strcat(pids_en_ready, "\0");
        }

        if (i != list_size(cola_ready_global->estado) - 1)
        {
            strcat(pids_en_ready, ",");
        }
    }
    return pids_en_ready;
}

// NEW -> READY
t_pcb *transicion_new_a_ready()
{
    t_pcb *proceso = transicion_generica(cola_new_global, cola_ready_global, "largo");
    sem_post(contador_readys);
    proceso->estado = READY;
    return proceso;
}
// EXEC -> EXIT
void *transicion_exit_largo_plazo()
{
    while (1)
    {
        sem_wait(b_largo_plazo_exit);
        transicion_generica(cola_exec_global, cola_exit_global, "exit_largo");
        sem_post(GRADO_MULTIPROGRAMACION);
        sem_post(b_exec_libre);
    }
}

// NEW/READY/BLOCKED -> EXIT (para finalizar)
void transicion_generica_exit(uint32_t pid)
{
    colaEstado *cola_encontrada = buscarCola(pid);
    t_pcb *pcb = sacar_proceso_cola(cola_encontrada, pid);
    agregar_a_estado(pcb, cola_exit_global);
    log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: EXIT \n", pid, cola_encontrada->nombreEstado);
    if (strcmp(cola_encontrada->nombreEstado, "NEW") == 0)
    {
        sem_post(GRADO_MULTIPROGRAMACION);
    }
    if (strcmp(cola_encontrada->nombreEstado, "EXEC") == 0)
    {
        sem_post(b_exec_libre);
    }
}

bool coincide_pid(t_pcb *pcb)
{
    return pcb->cde->pid == pid_a_sacar;
}

colaEstado *buscarCola(uint32_t pid)
{
    colaEstado *colas[5] = {cola_new_global, cola_ready_global, cola_ready_plus, cola_exec_global, cola_bloqueado_global};
    pid_a_sacar = pid;
    for (int i = 0; i < 5; i++)
    {
        if (list_any_satisfy(colas[i]->estado, coincide_pid))
            return colas[i];
    }
    return NULL;
}

t_pcb *sacar_proceso_cola(colaEstado *cola_estado, uint32_t pid)
{
    sem_wait(cola_estado->contador);
    pthread_mutex_lock(cola_estado->mutex_estado);
    pid_a_sacar = pid;
    t_pcb *pcb = list_remove_by_condition(cola_estado->estado, coincide_pid);
    log_info(logger, "PID: <%d> - SACADO DE COLA: <%s>", pcb->cde->pid, cola_estado->nombreEstado);
    pthread_mutex_unlock(cola_estado->mutex_estado);
    return pcb;
}