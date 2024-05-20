#include "../include/largoPlazo.h"

/*
Planificador de Largo Plazo
El Kernel será el encargado de gestionar las peticiones a la memoria para
la creación y eliminación de procesos. A continuación se detalla el comportamiento ante cada solicitud.


Eliminación de Procesos
Ante la llegada de un proceso al estado de EXIT (ya sea por solicitud de la CPU,
por un error, o por ejecución desde la consola del Kernel), el Kernel deberá
solicitar a la memoria que libere todas las estructuras asociadas al proceso y
marque como libre todo el espacio que este ocupaba.

En caso de que el proceso se encuentre ejecutando en CPU, se deberá enviar una
señal de interrupción a través de la conexión de interrupt con el mismo y aguardar
a que éste retorne el Contexto de Ejecución antes de iniciar la liberación de recursos.
Al eliminar un proceso se deberá habilitar 1 espacio en el grado de multiprogramación,
para que en caso de haber procesos encolados en NEW, el primero de estos pase a READY.
*/

/*
Creación de Procesos
Ante la solicitud de la consola de crear un nuevo proceso el
Kernel deberá informarle a la memoria que debe crear un proceso
cuyas operaciones corresponderán al archivo de pseudocódigo pasado
por parámetro, todos los procesos iniciarán sin espacio reservado en memoria,
por lo que solamente tendrán una tabla de páginas vacía.

En caso de que el grado de multiprogramación lo permita, los procesos creados
podrán pasar de la cola de NEW a la cola de READY, caso contrario, se quedarán
a la espera de que finalicen procesos que se encuentran en ejecución.

*/
void *largo_plazo()
{
    //log_info(logger, "--------------Planificador de Largo Plazo Iniciado-------------- \n");
    while (1)
    {

        sem_wait(b_reanudar_largo_plazo);
        

        sem_wait(binario_menu_lp); // Se bloquea esperando al menu

        sem_wait(GRADO_MULTIPROGRAMACION);

        t_pcb *proceso = malloc(sizeof(t_pcb));

        proceso = transicion_new_a_ready(); // lo saca de new y lo mete a ready

        log_info(logger, "PID: %d - Estado anterior: NEW - Estado actual: READY \n", proceso->cde->pid); // Log pedido de cambio de estado

        sem_post(procesos_en_ready);

        
    }
}
t_pcb *transicion_new_a_ready()
{
    t_pcb *proceso = malloc(sizeof(t_pcb));
    proceso->cde = malloc(sizeof(t_cde));
    proceso = sacar_procesos_cola(cola_new_global, procesos_en_new);

    agregar_a_estado(proceso, cola_ready_global, procesos_en_ready);
    return proceso;
}