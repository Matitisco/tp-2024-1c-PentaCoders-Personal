#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include <stdint.h>
#include "kernel.h"
#include "../include/cortoPlazo.h"

typedef enum motivoFinalizar
{
    SUCCESS,
    INVALID_RESOURCE,
    INVALID_WRITE
};
// extern uint32_t PID_GLOBAL;

void ejecutar_script(char *PATH);
void iniciar_proceso(char *PATH);
void finalizar_proceso(uint32_t PID);
void iniciar_planificacion();
void detener_planificacion();
void grado_multiprogramacion(int valor);
void listar_procesos_x_estado();
void renaudar_corto_plazo();
void renaudar_largo_plazo();
void pausar_corto_plazo();
void pausar_largo_plazo();
void modificar_grado_multiprogramacion(int valor);
void mostrar_procesos(colaEstado *cola);

// Funciones de busuqeda del proceso

uint32_t mostrarPID(t_pcb *proceso);
t_pcb *buscarProceso(uint32_t pid);
t_pcb *crear_proceso(char *PATH);
t_pcb *buscarPCBEnColaPorPid(int pid_buscado, t_queue *cola, char *nombreCola);
t_cde *iniciar_cde(char *PATH);

// char *mostrarMotivo(enum motivoFinalizar motivo);

// Funciones para liberar

void liberar_proceso(t_pcb *proceso);
void liberar_cde(t_pcb *proceso);
void liberar_recursos(t_pcb *proceso);
void liberar_archivos(t_pcb *proceso);

#endif