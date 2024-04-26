
#include "../include/kernel.h"
#include "../include/operaciones.h"

uint32_t PID_GLOBAL = 0;

void ejecutar_script()
{
    printf("Ejecutar Script");
}
// INICIAR PROCESO
void iniciar_proceso()
{
    t_pcb *proceso = crear_proceso();
    agregar_a_estado(proceso, cola_new_global);
    log_info(logger, "Se creo un proceso con PID %u en NEW", proceso->cde->pid);
}

t_pcb *crear_proceso()
{
    t_pcb *proceso_nuevo = malloc(sizeof(t_pcb)); // reservamos memoria para el proceso
    proceso_nuevo->quantum = 0;                   // por ahora en 0;
    proceso_nuevo->estado = NEW;                  // todo proceso empieza en NEW(ENUM)
    proceso_nuevo->cde = iniciar_cde();
    return proceso_nuevo;
}

t_cde *iniciar_cde()
{
    t_cde* cde = malloc(sizeof(t_cde));

    cde->pid = PID_GLOBAL;

    PID_GLOBAL++;

    cde->pc = 0; // LA CPU lo va a ir cambiando

    cde->registro = malloc(sizeof(t_registros));
    cde->registro = NULL;

    cde->instrucciones = malloc(sizeof(t_instruccion));
    cde->instrucciones = NULL;
    return cde;
 
}

void detener_proceso()
{
    printf("Detener Proceso");
}
void iniciar_planificacion()
{
    // tenemos un proceso en new y lo tenemos que pasar a ready
    // habilita a los hilos de los planificadores a que dejen de estar en pausa

    printf("Iniciar Planificacion");
}
void detener_planificacion()
{
    printf("Detener Planificacion");
}
void listar_procesos_x_estado()
{
    printf("Listar Procesos Por Estado");
}