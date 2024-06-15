#include "../include/largoPlazo.h"

char* plani_l= "largo";
void *largo_plazo()
{
    while (1)
    {


        sem_wait(binario_menu_lp); // Se bloquea esperando al menu

        sem_wait(GRADO_MULTIPROGRAMACION);
        t_pcb *proceso = malloc(sizeof(t_pcb));

        proceso = transicion_new_a_ready(); // lo saca de new y lo mete a ready

        log_info(logger, "PID: %d - Estado Anterior: NEW - Estado Actual: READY \n", proceso->cde->pid); // Log pedido de cambio de estado

        

    }
}





t_pcb *transicion_new_a_ready()
{
    /* t_pcb *proceso = sacar_procesos_cola(cola_new_global, plani_l);
     if(habilitar_planificadores==0)
	{
			sem_wait(b_reanudar_largo_plazo);
	} 
    agregar_a_estado(proceso, cola_ready_global); */

    t_pcb *proceso = transicion_generica(cola_new_global,cola_ready_global,"largo");

    return proceso;
}

void *transicion_exit_largo_plazo()
{
    while (1)
    {
        sem_wait(b_largo_plazo_exit);

        /* t_pcb *proceso = sacar_procesos_cola(cola_exec_global, plani_l);
        /* if(habilitar_planificadores==0)
	    {
			sem_wait(b_reanudar_largo_plazo);
		} 
        agregar_a_estado(proceso, cola_exit_global); */

        t_pcb *proceso = transicion_generica(cola_exec_global,cola_exit_global,"largo");

        sem_post(GRADO_MULTIPROGRAMACION);
        sem_post(b_exec_libre);

        log_info(logger, "Finaliza el proceso %d - Motivo:", proceso->cde->pid);
    }
}




