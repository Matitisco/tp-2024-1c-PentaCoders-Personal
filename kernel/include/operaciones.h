#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include <stdint.h>
#include "kernel.h"
#include "../include/cortoPlazo.h"

/* typedef enum motivoFinalizar{
    SUCCESS,
    INVALID_RESOURCE,
    INVALID_WRITE
}; */

/*Funciones del menu*/
void ejecutar_script();
void iniciar_proceso();
void finalizar_proceso(uint32_t PID);
void iniciar_planificacion();
void detener_planificacion();
void listar_procesos_x_estado();

/*Funciones de busuqeda del proceso*/
t_pcb *buscarPCBEnColaPorPid(int pid_buscado, t_queue *cola, char *nombreCola);


t_pcb *buscarProceso(uint32_t pid);
uint32_t mostrarPID(t_pcb *proceso);
t_pcb *crear_proceso();
t_cde *iniciar_cde();

//char *mostrarMotivo(enum motivoFinalizar motivo);

/*Funciones para liberar*/
void liberar_proceso(t_pcb *proceso);
void liberar_cde(t_pcb *proceso);
void liberar_recursos(t_pcb *proceso);
void liberar_archivos(t_pcb *proceso);

#endif