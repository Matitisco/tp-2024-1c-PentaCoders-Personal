#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include <stdint.h>
#include "kernel.h"
#include "../include/cortoPlazo.h"

// OPERACIONES
void ejecutar_script(char *PATH);
void iniciar_proceso(char *PATH);
void finalizar_proceso(uint32_t PID, motivoFinalizar motivo);
void finalizar_proceso_success(uint32_t PID, motivoFinalizar motivo);
void detener_planificacion();
void iniciar_planificacion();
void grado_multiprogramacion(int valor);
void proceso_estado();
void transicion_generica_exit(uint32_t pid);
// AUXILIARES
void renaudar_corto_plazo();
void renaudar_largo_plazo();
int cant_recursos_SO(t_recurso **recursos);
void pausar_corto_plazo();
void pausar_largo_plazo();
void modificar_grado_multiprogramacion(int valor);
void mostrar_procesos(colaEstado *cola);

colaEstado *obtener_cola(t_estados estado);
void imprimir_registro(void *element);
void destroy_archivos(void *element);
t_pcb *buscar_pcb_en_colas(int pid);
bool buscar_por_pid(t_pcb *proceso);
// Funciones de busuqeda del proceso
char *mostrar_motivo(motivoFinalizar motivo);
uint32_t mostrarPID(t_pcb *proceso);
t_pcb *buscarProceso(uint32_t pid);

t_pcb *crear_proceso(char *PATH);
t_pcb *buscarPCBEnColaPorPid(int pid_buscado, t_queue *cola, char *nombreCola);
t_pcb *buscarYSacarPCBEnColaPorPid(int buscado, t_queue *cola, char *nombreCola);
t_cde *iniciar_cde(char *PATH);

// LIBERAR RECURSOS/ARCHIVOS
void liberar_recursos(t_pcb *proceso);
void liberar_archivos(t_pcb *proceso);

void finalizar_proceso_final(t_pcb *proceso, int pid, motivoFinalizar motivo);

#endif
