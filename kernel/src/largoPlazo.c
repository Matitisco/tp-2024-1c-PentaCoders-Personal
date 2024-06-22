#include "../include/largoPlazo.h"

char *plani_l = "largo";

void *largo_plazo()
{
    while (1)
    {
        sem_wait(binario_menu_lp);
        sem_wait(GRADO_MULTIPROGRAMACION);

        t_pcb *proceso = transicion_new_a_ready();

        log_info(logger, "PID: %d - Estado Anterior: NEW - Estado Actual: READY \n", proceso->cde->pid);
    }
}
// NEW -> READY
t_pcb *transicion_new_a_ready()
{
    t_pcb *proceso = transicion_generica(cola_new_global, cola_ready_global, "largo");
    sem_post(contador_readys);
    return proceso;
}
// EXEC -> EXIT
void *transicion_exit_largo_plazo()
{
    while (1)
    {
        sem_wait(b_largo_plazo_exit);

        transicion_generica(cola_exec_global, cola_exit_global, "largo");

        sem_post(GRADO_MULTIPROGRAMACION);
        sem_post(b_exec_libre);
    }
}
