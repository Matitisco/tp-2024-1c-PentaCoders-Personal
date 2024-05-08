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

/* colaEstado *cola_new_global; // Declarar cada vez que lo usas
colaEstado *cola_ready_global;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;  */

void *largo_plazo()
{
    while (1)
    {

        log_info(logger, " \n Planificador de largo plazo iniciado \n");
        int *v1, *v2;
        v1 = malloc(sizeof(int));
        v2 = malloc(sizeof(int));
        sem_getvalue(procesos_en_new, v1);
        log_info(logger, " \n numero de procesos en new: %d \n", *v1);

        sem_wait(procesos_en_new); // Si hay procesos en NEW, avanza

        log_info(logger, " \n Paso Primer wait \n");

        sem_getvalue(GRADO_MULTIPROGRAMACION, v2);
        log_info(logger, " \n numero de G.MULTIPROGRAMACION en new: %d \n", *v2);

        sem_wait(GRADO_MULTIPROGRAMACION); // Si el grado de multiporgrmacion lo permite. Ponerlo global para que lo conozca todo el kernel
        // poner un signal   en exit
        log_info(logger, " \n Paso Segundo wait \n");

        t_pcb *proceso = malloc(sizeof(t_pcb));
        proceso->cde = malloc(sizeof(t_cde));
        transicion_new_a_ready(proceso); // lo saca de new y lo mete a ready

        log_info(logger, "PID: %d - Estado anterior: NEW - Estado actual: READY \n", proceso->cde->pid); // Log pedido de cambio de estado

        sem_post(procesos_en_ready);
    }
}

void transicion_new_a_ready(t_pcb *proceso)
{
    log_info(logger, " \n LLEGO A TRANSICION NEW->READY \n");

    sacar_procesos_cola(proceso, cola_new_global, procesos_en_new);  // saco el proceso de la cola de new
    agregar_a_estado(proceso, cola_ready_global, procesos_en_ready); // lo agrego a la cola de ready

    log_info(logger, " \n SALIO DE TRANSICION NEW->READY \n");

}